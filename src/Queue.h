#ifndef INC_QUEUE_H
#define INC_QUEUE_H
#include <vector>
#include <string>
/// Hold information on a particular queue/partition
class Queue {
  public:
    typedef std::vector<std::string> Sarray;
    /// Queue system type. Keep in sync with TypeStr_
    enum Type { PBS = 0, SLURM, NO_QUEUE };
    /// CONSTRUCTOR
    Queue();
    /// Print help to stdout
    static void OptHelp();
    /// Process queue-specific option
    int ParseOption(std::string const&, std::string const&);
    /// Print info to stdout
    void Info() const;
    /// \return True if queue has enough info for job submission
    bool IsValid() const;
    /// \return header text for queue flags
    const char* HeaderFlag() const;
    /// \return Queue submit command
    const char* SubmitCmd() const;
  private:
    /// KEEP IN SYNC with Type
    static const char* TypeStr_[];

    std::string key_;           ///< String uniquely identifying this queue
    std::string name_;          ///< Queue/partition name
    int ppn_;                   ///< Processors per node
    Sarray additionalCommands_; ///< Additional script commands needed for this queue
    Sarray Flags_;              ///< Additional queue flags
    Type queueType_;            ///< Queueing system type
};
#endif
