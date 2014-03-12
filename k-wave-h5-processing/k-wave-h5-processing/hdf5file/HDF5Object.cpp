#include "HDF5Object.h"
#include "HDF5Attribute.h"

HDF5File::HDF5Object::HDF5Object(H5::H5Object *_object)
{
    object = _object;
}

HDF5File::HDF5Object::~HDF5Object()
{

}

int HDF5File::HDF5Object::getNumAttrs()
{
    return object->getNumAttrs();
}

HDF5File::HDF5Object::HDF5Attribute *HDF5File::HDF5Object::getAttribute(H5std_string name)
{
    H5::Attribute attr;
    try {
        std::cout << "Getting attribute \"" << name << "\"";
        attr = object->openAttribute(name);
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
    HDF5Attribute *at = new HDF5Attribute(attr);
    attr.close();
    return at;
}

HDF5File::HDF5Object::HDF5Attribute *HDF5File::HDF5Object::getAttribute(const unsigned int idx)
{
    H5::Attribute attr;
    try {
        std::cout << "Getting attribute " << idx;
        attr = object->openAttribute(idx);
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
    HDF5Attribute *at = new HDF5Attribute(attr);
    attr.close();
    return at;
}

void HDF5File::HDF5Object::removeAttribute(const unsigned int idx)
{
    try {
        std::cout << "Removing attribute " << idx;
        object->removeAttr(object->openAttribute(idx).getName());
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
    }
}

void HDF5File::HDF5Object::removeAttribute(const H5std_string name)
{
    try {
        std::cout << "Removing attribute \"" << name << "\"";
        object->removeAttr(name);
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
    }
}

void HDF5File::HDF5Object::setAttribute(HDF5Attribute *attribute)
{
    try {
        object->removeAttr(attribute->getName());
    } catch(H5::AttributeIException error) {

    }

    try {
        std::cout << "Creating attribute \"" << attribute->getName() << "\"";
        H5::Attribute att = object->createAttribute(attribute->getName(), attribute->getDataType(), attribute->getSpace());
        att.write(attribute->getDataType() , attribute->getData());
        att.close();
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

void HDF5File::HDF5Object::setAttribute(const H5std_string name, int value)
{
    try {
        object->removeAttr(name);
    } catch(H5::AttributeIException error) {

    }

    try {
        std::cout << "Creating int attribute \"" << name << " = " << value << "\"";
        H5::IntType type(H5::PredType::NATIVE_INT);
        H5::DataSpace att_space(H5S_SCALAR);
        H5::Attribute att = object->createAttribute(name, type, att_space);
        att.write(type, &value);
        att.close();
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

void HDF5File::HDF5Object::setAttribute(const H5std_string name, uint64_t value)
{
    try {
        object->removeAttr(name);
    } catch(H5::AttributeIException error) {

    }

    try {
        std::cout << "Creating uint64_t attribute \"" << name << " = " << value << "\"";
        H5::IntType type(H5::PredType::NATIVE_UINT64);
        H5::DataSpace att_space(H5S_SCALAR);
        H5::Attribute att = object->createAttribute(name, type, att_space);
        att.write(type, &value);
        att.close();
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

void HDF5File::HDF5Object::setAttribute(const H5std_string name, double value)
{
    try {
        object->removeAttr(name);
    } catch(H5::AttributeIException error) {

    }

    try {
        std::cout << "Creating double attribute \"" << name << " = " << value << "\"";
        H5::IntType type(H5::PredType::NATIVE_DOUBLE);
        H5::DataSpace att_space(H5S_SCALAR);
        H5::Attribute att = object->createAttribute(name, type, att_space);
        att.write(type, &value);
        att.close();
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

void HDF5File::HDF5Object::setAttribute(const H5std_string name, float value)
{
    try {
        object->removeAttr(name);
    } catch(H5::AttributeIException error) {

    }

    try {
        std::cout << "Creating float attribute \"" << name << " = " << value << "\"";
        H5::IntType type(H5::PredType::NATIVE_FLOAT);
        H5::DataSpace att_space(H5S_SCALAR);
        H5::Attribute att = object->createAttribute(name, type, att_space);
        att.write(type, &value);
        att.close();
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

void HDF5File::HDF5Object::setAttribute(const H5std_string name, const H5std_string value)
{
    try {
        object->removeAttr(name);
    } catch(H5::AttributeIException error) {

    }

    try {
        std::cout << "Creating string attribute \"" << name << " = " << value << "\"";
        H5::StrType type(0, H5T_VARIABLE);
        H5::DataSpace att_space(H5S_SCALAR);
        H5::Attribute att = object->createAttribute(name, type, att_space);
        const char *str = value.c_str();
        att.write(type, &str);
        att.close();
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

float HDF5File::HDF5Object::readAttributeF(const H5std_string name)
{
    float value;
    try {
        std::cout << "Reading float attribute \"" << name << "\"";
        H5::FloatType type(H5::PredType::NATIVE_FLOAT);
        object->openAttribute(name).read(type, &value);
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
    return value;
}

uint64_t HDF5File::HDF5Object::readAttributeI(const H5std_string name)
{
    uint64_t value;
    try {
        std::cout << "Reading uint64_t attribute \"" << name << "\"";
        H5::IntType type(H5::PredType::NATIVE_UINT64);
        object->openAttribute(name).read(type, &value);
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
    return value;
}

H5std_string HDF5File::HDF5Object::readAttributeS(const H5std_string name)
{
    char *value;
    H5std_string retValue;
    try {
        std::cout << "Reading string attribute \"" << name << "\"";
        H5::StrType type = object->openAttribute(name).getStrType();
        if (type.isVariableStr()) {
            object->openAttribute(name).read(type, &value);
            retValue = H5std_string(value);
        } else {
            value = new char[type.getSize()]();
            object->openAttribute(name).read(type, value);
            retValue = H5std_string(value);
            delete [] value;
        }
        std::cout << " ... OK" << std::endl;
    } catch(H5::DataTypeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
    return retValue;
}

