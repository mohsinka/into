/* This file is part of Into.
 * Copyright (C) Intopii 2013.
 * All rights reserved.
 *
 * Licensees holding a commercial Into license may use this file in
 * accordance with the commercial license agreement. Please see
 * LICENSE.commercial for commercial licensing terms.
 *
 * Alternatively, this file may be used under the terms of the GNU
 * Affero General Public License version 3 as published by the Free
 * Software Foundation. In addition, Intopii gives you special rights
 * to use Into as a part of open source software projects. Please
 * refer to LICENSE.AGPL3 for details.
 */

#ifndef _PIIDEFAULTOPERATION_H
#define _PIIDEFAULTOPERATION_H

#include <QList>
#include <QStringList>
#include <PiiReadWriteLock.h>
#include "PiiBasicOperation.h"
#include "PiiFlowController.h"

class PiiOperationProcessor;

/**
 * An easy-to-use implementation of the PiiOperation interface. This
 * class implements synchronization for branching and joining
 * processing pipelines and provides different threading models for
 * running operations in non-threaded, single-threaded and
 * multi-threaded modes. PiiDefaultOperation also ensures mutual
 * exclusion between processing and property changes so that one can
 * safely set operation properties while it is running.
 *
 * PiiDefaultOperation delegates the task of handling synchronization
 * issues to PiiFlowController. It uses different flow controllers for
 * different operations to optimize the performance of passing
 * objects. The threading mode of the operation is controlled by the
 * [threadCount] property.
 *
 */
class PII_YDIN_EXPORT PiiDefaultOperation :
  public PiiBasicOperation,
  protected PiiFlowController::SyncListener
{
  Q_OBJECT

  /**
   * The number of threads reserved for executing this operation. If
   * this property is set to a non-zero value, at least one thread
   * will always be reserved.
   *
   * If `threadCount` is zero, objects are processed immediately when
   * received in the context of the sending thread. If many inputs are
   * connected, the thread calling [process()] and [syncEvent()] may be
   * different from time to time, but no concurrent calls will be
   * made. This is the default value.
   *
   * If `threadCount` is one, processing happens in a separate thread
   * that is awakened when new objects appear. The thread calling
   * [process()], [syncEvent()] is always the same, and no concurrent
   * calls will be made.
   *
   * if `threadCount` is greater than one, a pool of threads will be
   * created. The system ensures that [syncEvent()] and setProperty()
   * are always called in isolation, but calls to [process()] may
   * happen simultaneously in any of these threads.
   *
   * ! If `threadCount` is larger than one, special attention is
   * required. If the state of the operation is altered in [process()],
   * one needs to ensure that mutual exclusion is handled properly.
   * For example, counters may need to be implemented with QAtomicInt.
   * Temporary variables that cache previous processing results need
   * to made thread-specific. Furthermore, strict ordering limits (see
   * PiiDefaultFlowController) won't apply. The threads will be
   * started in correct order, but execution turns are given by the
   * operating system, and strict ordering won't be guaranteed. For
   * the same reason, operations with no input sockets will emit their
   * processing results in an arbitrary order. If such an operation
   * stops spontaneously in one thread, it must be prepared for the
   * fact that other threads may be still running in process().
   *
   * The number of threads can only be changed when the operation is
   * either stopped or paused, and only before [check()]. Setting the
   * value in other situations has no effect. Furthermore, some
   * derived operations may disable changes to the property
   * altogether.
   */
  Q_PROPERTY(int threadCount READ threadCount WRITE setThreadCount);

  /**
   * The priority of the operation when [threadCount] is non-zero.
   * Threaded operations with a high priority are more likely to be
   * scheduled for execution than those with a low priority. Use
   * QThread::Priority as the value for this property. The default
   * value is QThread::NormalPriority. Note that this value has no
   * effect on non-threaded operations.
   *
   * Within a single processing pipeline, priority is meaningful only
   * for asynchronous operations that may discard some of their input
   * objects. Setting priority to a lower value increases the
   * probability of discarding an input object. Priority is useful for
   * synchronous operations if a configuration contains multiple
   * independent processing pipelines or branching pipelines.
   *
   * ! Thread priority cannot be changed on Linux.
   */
  Q_PROPERTY(int priority READ priority WRITE setPriority);

  /**
   * This property lists the threading modes the operation is allowed
   * to run in. The default value is `NonThreaded |
   * SingleThreaded`. Subclasses may change the default if they
   * support `MultiThreaded` mode or cannot support either of the
   * default modes.
   */
  Q_PROPERTY(ThreadingCapabilities threadingCapabilities READ threadingCapabilities);
  Q_FLAGS(ThreadingCapabilities);

public:
  typedef PiiFlowController::SyncEvent SyncEvent;

  /**
   * Threading capabilities.
   *
   * - `NonThreaded` - the operation allows setting [threadCount] to
   * 0.
   *
   * - `SingleThreaded` - the operation allows setting [threadCount]
   * to 1.
   *
   * - `MultiThreaded` - the operation allows setting [threadCount] to
   * a value greater than one. This kind of operations are prepared
   * for concurrent process() calls.
   */
  enum ThreadingCapability { NonThreaded = 1, SingleThreaded = 2, MultiThreaded = 4 };
  Q_DECLARE_FLAGS(ThreadingCapabilities, ThreadingCapability);

  PiiDefaultOperation();
  ~PiiDefaultOperation();

  /**
   * Ensures that no property will be set while process() or
   * syncEvent() is being called by acquiring [processLock()] for
   * writing. Then sets the property.
   */
  bool setProperty(const char* name, const QVariant& value);

  /**
   * Acquires [processLock()] for reading and returns the property.
   */
  QVariant property(const char* name) const;

  /**
   * Checks the operation for execution. This function creates a
   * suitable flow controller by calling [createFlowController()]. It
   * then sets the flow controller to the active
   * [processor](PiiOperationProcessor) and sets the processor as the
   * [input controller](PiiInputController) for all inputs. It
   * also makes all output sockets listeners to their connected
   * inputs.
   *
   * If you change socket groupings in your overridden implementation,
   * call PiiDefaultOperation::check() **after** that. Otherwise, your
   * new groupings will not be in effect. If you create a custom
   * listener for inputs connected to the outputs of this operation,
   * install it *after* calling this function.
   */
  void check(bool reset);

  /**
   * Starts the processor. If [check()] has not been called, this
   * function writes out a warning and returns.
   */
  void start();

  /**
   * Interrupts the processor. Does not alter the state of the
   * operation unless the processor does so.
   */
  void interrupt();

  /**
   * Prepares the operation for pausing. The functioning is dependent
   * on the type of the processor. In threaded mode, the operation
   * will be turned into `Pausing` state, and processing will pause
   * once the thread has finished its current processing round. In
   * non-threaded mode, the processor will check if the operation has
   * connected inputs. If it does, the operation will turn into
   * `Pausing` state and wait until it receives pause signals from
   * previous operations in the pipeline. If there are no connected
   * inputs, the operation will immediately turn into `Paused` state.
   */
  void pause();

  /**
   * Applies cached property changes. If the operation is either
   * `Paused` or `Stopped`, sets the properties directly. Otherwise it
   * works similarly to pause(), but it instead of changing the state
   * of the operation it changes applies the given property set.
   */
  void reconfigure(const QString& propertySetName = QString());

  /**
   * Prepares the operation for stopping. Works analogously to
   * [pause()].
   */
  void stop();

  /**
   * Waits until the processor is finished.
   */
  bool wait(unsigned long time = ULONG_MAX);

protected:
  /// @internal
  class PII_YDIN_EXPORT Data : public PiiBasicOperation::Data
  {
  public:
    Data();
    ~Data();

  private:
    friend class PiiDefaultOperation;
    friend class PiiSimpleProcessor;
    friend class PiiThreadedProcessor;
    friend class PiiMultiThreadedProcessor;

    // Handles object flow. Synchronizes inputs etc.
    PiiFlowController* pFlowController;

    // Executes process() when needed.
    PiiOperationProcessor* pProcessor;

    // The group id of the input group being processed.
    int iActiveInputGroup;

    bool bChecked;

    mutable PiiReadWriteLock processLock;
    int iThreadCount;
    ThreadingCapabilities threadingCapabilities;
  };
  PII_D_FUNC;

  /// @internal
  PiiDefaultOperation(Data* data);

  void setThreadCount(int threadCount);
  int threadCount() const;

  void setPriority(int priority);
  int priority() const;

  void setThreadingCapabilities(ThreadingCapabilities threadingCapabilities);
  ThreadingCapabilities threadingCapabilities() const;

  /**
   * Executes one round of processing. This function is invoked by the
   * processor if the necessary preconditions for a new processing
   * round are met. This function does all the necessary calculations
   * to create output objects and sends them to output sockets.
   *
   * Calls to process(), syncEvent(), and setProperty() are
   * synchronized and cannot occur simultaneously.
   * PiiDefaultOperation ensures this by locking [processLock()] for
   * reading before calling process(). If [threadCount] is set to a
   * value larger than one, process() may be called simultaneously
   * from many threads, but neither syncEvent() nor setProperty() will
   * ever overlap it.
   *
   * **Note:** With time-consuming operations, one should occasionally
   * check that the operation hasn't been interrupted, i.e. that
   * state() returns `Running`.
   *
   * @exception PiiExecutionException whenever an unrecoverable error
   * occurs during a processing round, the operation is interrupted,
   * or finishes execution due to end of input data.
   */
  virtual void process() = 0;

  /**
   * Returns the id of the synchronized socket group being processed.
   * If all input sockets work in parallel, or there are no input
   * sockets, this value can be safely ignored. Otherwise, one can use
   * the value to decide which sockets need to be read and processed
   * in [process()].
   */
  int activeInputGroup() const;

  /**
   * Informs the operation about synchronization events. The most
   * typical use of this function is to see when all objects in an
   * input group and all of its child groups have been received. For
   * example, if the operation reads large images from one input and a
   * number of sub-images for each large image from another input, a
   * sync event is sent whenever all the small images that correspond
   * to one large image have been processed. Your implementation may
   * then either just record the synchronized state or to send any
   * buffered data. The default implementation does nothing.
   *
   * Calls to process(), syncEvent(), and setProperty() are
   * synchronized and cannot occur simultaneously.
   * PiiDefaultOperation ensures this by locking [processLock()] for
   * reading before calling syncEvent().
   *
   * When entering syncEvent(), input sockets are in undefined state.
   * If you need data from the inputs in syncEvent(), you need to
   * store the objects in process().
   *
   * ~~~(c++)
   * void MyOperation::syncEvent(SyncEvent* event)
   * {
   *   if (event->type() == SyncEvent::EndInput &&
   *       event->groupId() == _pLargeImageInput->groupId())
   *     doWhateverNeededNow();
   * }
   * ~~~
   */
  void syncEvent(SyncEvent* event);

  /**
   * Creates a flow controller for this operation. This function is
   * called by the default implementation of [check()]. A new flow
   * controller will be always be created when the [check()] function
   * is called. The old controller will be deleted.
   *
   * The default implementation tries to find an optimal flow
   * controller for the active input configuration:
   *
   * - If there are no connected inputs, a null pointer will be
   * returned.
   *
   * - If the operation has only one connected input,
   * PiiOneInputFlowController will be used.
   *
   * - If there are many inputs, but all are in the same group,
   * PiiOneGroupFlowController will be used.
   *
   * - Otherwise, PiiDefaultFlowController will be used. The flow
   * controller will be configured with *loose* parent-child
   * relationships between groups with a non-negative group id and at
   * least one connected socket. The relationships will be assigned in
   * the order of increasing magnitude. For example, if there are
   * sockets in groups 0, 1, and -1, 0 will be set as the (loose)
   * parent of 1. Group -1 will be independent of the others.
   *
   * Subclasses may override the default behavior by installing a
   * custom flow controller. You need to do this if your operation
   * has, for example, sibling groups that share a common parent
   * group, or if you need to assign *strict* relationships between
   * input groups. If you override this function, make sure the flow
   * controller takes the control of all connected inputs.
   *
   * @return a pointer to a flow controller. PiiDefaultOperation will
   * take the ownership of the pointer. If you don't want inputs to be
   * controlled, return 0.
   *
   * @see PiiDefaultFlowController
   */
  virtual PiiFlowController* createFlowController();

  /**
   * Returns `true` if the operation has been checked for execution
   * ([check()]) but not started ([start()]) yet.
   */
  bool isChecked() const;

  /**
   * Returns a pointer to a lock that PiiDefaultOperation uses to
   * synchronize calls to [property()], [setProperty()], [process()], and
   * [syncEvent()]. This lock can be used if a subclass needs to
   * protect stuff from being accessed concurrently. Acquiring the
   * lock for writing blocks concurrent calls to all of the mentioned
   * functions. Acquiring the lock for reading blocks only
   * [setProperty()] and allows simultaneous execution of the other
   * (read-locked) functions.
   */
  PiiReadWriteLock* processLock();

private:
  void init();
  void createProcessor();

  friend class PiiSimpleProcessor;
  friend class PiiThreadedProcessor;
  friend class PiiMultiThreadedProcessor;

  inline void processLocked()
  {
    PiiReadLocker lock(&_d()->processLock);
    process();
  }

  inline void sendSyncEvents(PiiFlowController* controller)
  {
    PiiReadLocker lock(&_d()->processLock);
    controller->sendSyncEvents(this);
  }

  inline bool isAcceptableThreadCount(int threadCount) const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PiiDefaultOperation::ThreadingCapabilities);

#endif //_PIIDEFAULTOPERATION_H
