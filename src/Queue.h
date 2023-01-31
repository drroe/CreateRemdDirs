#ifndef INC_QUEUE_H
#define INC_QUEUE_H
#include <vector>
#include <string>
/// Hold information on a particular queue/partition
class Queue {
  public:
    typedef std::vector<std::string> Sarray;
    /// Queue system type
    enum Type { PBS = 0, SLURM, NO_QUEUE }

    Queue();
  private:
    std::string key_;           ///< String uniquely identifying this queue
    std::string name_;          ///< Queue/partition name
    int ppn_;                   ///< Processors per node
    Sarray additionalCommands_; ///< Additional script commands needed for this queue
    Sarray Flags_;              ///< Additional queue flags
    Type queueType_;            ///< Queueing system type
};
#endif
