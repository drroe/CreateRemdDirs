#ifndef INC_QUEUEARRAY_H
#define INC_QUEUEARRAY_H
#include <vector>
class Queue;
/// Hold information on all available queues
class QueueArray {
  public:
    /// CONSTRUCTOR
    QueueArray();
  private:
    typedef std::vector<Queue> Qarray;

    Qarray queues_;
};
#endif
