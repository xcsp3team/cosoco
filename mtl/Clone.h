#ifndef Cosoco_Clone_h
#define Cosoco_Clone_h


namespace Cosoco {

class Clone {
   public:
    virtual Clone* clone() const = 0;
};
};   // namespace Cosoco

#endif