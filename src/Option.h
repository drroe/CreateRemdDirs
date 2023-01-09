#ifndef INC_OPTION_H
#define INC_OPTION_H
template <class T> class Option {
  public:
    Option(T const& defaultVal) : val_(defaultVal), defaultVal_(defaultVal), isSet_(false) {}
    T const& Val() const { return val_; }
    bool IsSet() const { return isSet_; }
    void SetVal(T const& v) { val_ = v; isSet_ = true; }
  private:
    T val_; ///< Option value
    T defaultVal_; ///< Option default value
    bool isSet_; ///< True if option has been explicitly set.
};
#endif
