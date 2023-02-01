#ifndef INC_QUEUE_H
#define INC_QUEUE_H
#include <vector>
#include <string>
class TextFile;
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
    std::string HeaderFlag() const;
    /// \return Queue submit command
    std::string SubmitCmd() const;
    /// Write additional queue flags to file
    int AdditionalFlags(TextFile&) const;

    /// \return Key
    std::string const& Key() const { return key_; }
    std::string const& Name() const { return name_; }
    int PPN() const { return ppn_; }
    Sarray const& AdditionalCommands() const { return additionalCommands_; }
    Type QueueType() const { return queueType_; }
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
