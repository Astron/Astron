// Filename: NumericType.h
#pragma once
#include "NumericRange.h"
#include "DistributedType.h"
namespace dclass   // open namespace dclass
{


// A NumericType can represent any of the basic number types (ie. integers, floats, etc).
//     A NumericType may also have a range and/or modulus to limit its possible values,
//     and/or a divisor representing a fixed-point numeric convention.
//     A divisor scales up any range or modulus to constrain up to (constraint * divisor).
class NumericType : public DistributedType
{
  public:
    // Type constructor
    NumericType(Type type);

    // as_numeric returns this as a NumericType if it is numeric, or nullptr otherwise.
    virtual NumericType* as_numeric();
    virtual const NumericType* as_numeric() const;

    // get_divisor returns the divisor of the numeric, with a default value of one.
    inline unsigned int get_divisor() const;

    // has_modulus returns true if the numeric is constrained by a modulus.
    inline bool has_modulus() const;
    // get_modulus returns a double representation of the modulus value.
    inline double get_modulus() const;

    // has_range returns true if the numeric is constrained by a range.
    virtual bool has_range() const;
    // get_range returns the NumericRange that constrains the type's values.
    inline NumericRange get_range() const;

    // set_divisor sets a divisor for the numeric type, typically to represent fixed-point.
    //     Returns false if the divisor is not valid for this type.
    bool set_divisor(unsigned int divisor);
    // set_modulus sets a modulus value of the numeric type.
    //     Returns false if the modulus is not valid for this type.
    bool set_modulus(double modulus);
    // set_range sets a valid range of the numeric type.
    //     Returns false if the range is not valid for this type.
    bool set_range(const NumericRange &range);

    virtual bool within_range(const std::vector<uint8_t>* data, uint64_t length) const;

    // generate_hash accumulates the properties of this type into the hash.
    virtual void generate_hash(HashGenerator &hashgen) const;

  private:
    unsigned int m_divisor;

    std::pair<bool, Number> data_to_number(const std::vector<uint8_t>* data) const;

    // These are the original range and modulus values from the file, unscaled by the divisor.
    double m_orig_modulus;
    NumericRange m_orig_range;

    // These are the range and modulus values after scaling by the divisor.
    Number m_modulus;
    NumericRange m_range;
};


} // close namespace dclass
#include "NumericType.ipp"
