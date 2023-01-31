#ifndef INC_MDINTERFACE_H
#define INC_MDINTERFACE_H
class MdPackage;
/// Wrapper around MdPackage
class MdInterface {
  public:
    enum Type { AMBER = 0, UNKNOWN_PACKAGE };
    /// CONSTRUCTOR
    MdInterface();
    /// DESTRUCTOR
    ~MdInterface();
    /// COPY CONSTRUCTOR
    MdInterface(MdInterface const&);
    /// ASSIGNMENT
    MdInterface& operator=(MdInterface const&);

    /// Allocate package with given debug level
    int AllocatePackage(Type, int);
    /// Set package debug level
    void SetDebug(int) const;
    /// \return allocated package
    MdPackage* Package() const { return package_; }
    /// \return True if a package is allocated
    bool HasPackage() const { return (package_ != 0); }
  private:
    MdPackage* package_;
    Type type_;
};
#endif
