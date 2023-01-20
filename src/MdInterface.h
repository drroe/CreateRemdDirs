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

    /// Allocate package
    int AllocatePackage(Type);
    /// \return allocated package
    MdPackage* Package() const { return package_; }
  private:
    MdPackage* package_;
    Type type_;
};
#endif
