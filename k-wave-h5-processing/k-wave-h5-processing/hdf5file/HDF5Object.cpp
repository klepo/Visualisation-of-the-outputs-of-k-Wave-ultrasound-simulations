#include "HDF5Object.h"
#include "HDF5Attribute.h"

HDF5File::HDF5Object::HDF5Object(H5::H5Object *object)
{
    this->object = object;
}

HDF5File::HDF5Object::~HDF5Object()
{

}

int HDF5File::HDF5Object::getNumAttrs()
{
    //mutex.lock();
    int num = (int) object->getNumAttrs();
    //mutex.unlock();
    return num;
}

bool HDF5File::HDF5Object::hasAttribute(H5std_string name)
{
    //return ((H5::H5Location *) object)->attrExists(name);
    try {
        object->openAttribute(name);
        return true;
    } catch(H5::Exception) {
        return false;
    }
}

HDF5File::HDF5Object::HDF5Attribute *HDF5File::HDF5Object::getAttribute(H5std_string name)
{
    H5::Attribute attr;
    try {
        //mutex.lock();
        std::cout << "Getting attribute \"" << name << "\"";
        attr = object->openAttribute(name);
        std::cout << " ... OK" << std::endl;
        //mutex.unlock();
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        //mutex.unlock();
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
        //mutex.lock();
        std::cout << "Getting attribute " << idx;
        attr = object->openAttribute(idx);
        std::cout << " " << attr.getName();
        std::cout << " ... OK" << std::endl;
        //mutex.unlock();
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        //mutex.unlock();
        throw std::runtime_error(error.getCDetailMsg());
    }
    HDF5Attribute *at = new HDF5Attribute(attr);
    attr.close();
    return at;
}

void HDF5File::HDF5Object::removeAttribute(const unsigned int idx)
{
    //mutex.lock();
    try {
        std::cout << "Removing attribute " << idx;
        object->removeAttr(object->openAttribute(idx).getName());
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
    }
    //mutex.unlock();
}

void HDF5File::HDF5Object::removeAttribute(const H5std_string name)
{
    //mutex.lock();
    try {
        std::cout << "Removing attribute \"" << name << "\"";
        object->removeAttr(name);
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
    }
    //mutex.unlock();
}

void HDF5File::HDF5Object::setAttribute(HDF5Attribute *attribute)
{
    //mutex.lock();
    try {
        object->removeAttr(attribute->getName());
    } catch(H5::AttributeIException error) {

    }
    //mutex.unlock();

    try {
        //mutex.lock();
        std::cout << "Creating attribute \"" << attribute->getName() << "\"";
        H5::Attribute att = object->createAttribute(attribute->getName(), attribute->getDataType(), attribute->getSpace());
        att.write(attribute->getDataType() , attribute->getData());
        att.close();
        std::cout << " ... OK" << std::endl;
        //mutex.unlock();
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        //mutex.unlock();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

void HDF5File::HDF5Object::setAttribute(const H5std_string name, int value)
{
    //mutex.lock();
    try {
        object->removeAttr(name);
    } catch(H5::AttributeIException error) {

    }
    //mutex.unlock();

    try {
        //mutex.lock();
        std::cout << "Creating int attribute \"" << name << " = " << value << "\"";
        H5::IntType type(H5::PredType::NATIVE_INT);
        H5::DataSpace att_space(H5S_SCALAR);
        H5::Attribute att = object->createAttribute(name, type, att_space);
        att.write(type, &value);
        att.close();
        std::cout << " ... OK" << std::endl;
        //mutex.unlock();
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        //mutex.unlock();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

void HDF5File::HDF5Object::setAttribute(const H5std_string name, uint64_t value)
{
    //mutex.lock();
    try {
        object->removeAttr(name);
    } catch(H5::AttributeIException error) {

    }
    //mutex.unlock();

    try {
        //mutex.lock();
        std::cout << "Creating uint64_t attribute \"" << name << " = " << value << "\"";
        H5::IntType type(H5::PredType::NATIVE_UINT64);
        H5::DataSpace att_space(H5S_SCALAR);
        H5::Attribute att = object->createAttribute(name, type, att_space);
        att.write(type, &value);
        att.close();
        std::cout << " ... OK" << std::endl;
        //mutex.unlock();
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        //mutex.unlock();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

void HDF5File::HDF5Object::setAttribute(const H5std_string name, double value)
{
    //mutex.lock();
    try {
        object->removeAttr(name);
    } catch(H5::AttributeIException error) {

    }
    //mutex.unlock();

    try {
        //mutex.lock();
        std::cout << "Creating double attribute \"" << name << " = " << value << "\"";
        H5::IntType type(H5::PredType::NATIVE_DOUBLE);
        H5::DataSpace att_space(H5S_SCALAR);
        H5::Attribute att = object->createAttribute(name, type, att_space);
        att.write(type, &value);
        att.close();
        std::cout << " ... OK" << std::endl;
        //mutex.unlock();
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        //mutex.unlock();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

void HDF5File::HDF5Object::setAttribute(const H5std_string name, float value)
{
    //mutex.lock();
    try {
        object->removeAttr(name);
    } catch(H5::AttributeIException error) {

    }
    //mutex.unlock();

    try {
        //mutex.lock();
        std::cout << "Creating float attribute \"" << name << " = " << value << "\"";
        H5::IntType type(H5::PredType::NATIVE_FLOAT);
        H5::DataSpace att_space(H5S_SCALAR);
        H5::Attribute att = object->createAttribute(name, type, att_space);
        att.write(type, &value);
        att.close();
        std::cout << " ... OK" << std::endl;
        //mutex.unlock();
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        //mutex.unlock();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

void HDF5File::HDF5Object::setAttribute(const H5std_string name, const H5std_string value)
{
    //mutex.lock();
    try {
        object->removeAttr(name);
    } catch(H5::AttributeIException error) {

    }
    //mutex.unlock();

    try {
        //mutex.lock();
        std::cout << "Creating string attribute \"" << name << " = " << value << "\"";
        H5::StrType type(0, H5T_VARIABLE);
        H5::DataSpace att_space(H5S_SCALAR);
        H5::Attribute att = object->createAttribute(name, type, att_space);
        const char *str = value.c_str();
        att.write(type, &str);
        att.close();
        std::cout << " ... OK" << std::endl;
        //mutex.unlock();
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        //mutex.unlock();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

float HDF5File::HDF5Object::readAttributeF(const H5std_string name)
{
    float value;
    try {
        mutex.lock();
        std::cout << "Reading float attribute \"" << name << "\"";
        H5::FloatType type(H5::PredType::NATIVE_FLOAT);
        object->openAttribute(name).read(type, &value);
        std::cout << " ... OK" << std::endl;
        mutex.unlock();
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        mutex.unlock();
        throw std::runtime_error(error.getCDetailMsg());
    }
    return value;
}

uint64_t HDF5File::HDF5Object::readAttributeI(const H5std_string name)
{
    uint64_t value;
    try {
        mutex.lock();
        std::cout << "Reading uint64_t attribute \"" << name << "\"";
        H5::IntType type(H5::PredType::NATIVE_UINT64);
        object->openAttribute(name).read(type, &value);
        std::cout << " ... OK" << std::endl;
        mutex.unlock();
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        mutex.unlock();
        throw std::runtime_error(error.getCDetailMsg());
    }
    return value;
}

H5std_string HDF5File::HDF5Object::readAttributeS(const H5std_string name)
{
    char *value;
    H5std_string retValue;
    try {
        mutex.lock();
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
        mutex.unlock();
    } catch(H5::DataTypeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        mutex.unlock();
        throw std::runtime_error(error.getCDetailMsg());
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        mutex.unlock();
        throw std::runtime_error(error.getCDetailMsg());
    }
    return retValue;
}

