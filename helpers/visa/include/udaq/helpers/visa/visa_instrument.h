#pragma once

#include <vector>

#include <visa.h>

#include <udaq/types.h>
#include <udaq/helpers/visa/visa_exception.h>

namespace udaq::visa {

typedef std::string visa_address_t;

class visa_instrument {
  public:
    visa_instrument() {
        auto status= viOpenDefaultRM (&resource_manager);
        if ( status < VI_SUCCESS)
            throw udaq::visa::VisaException(status);
    }
    ~visa_instrument() {
        viClose(resource_manager);
    }

    /* Connects to the instrument. */
    virtual void connect(const visa_address_t& address)
    {
        this->address = address;
        auto status = viOpen (resource_manager,  address.c_str(), VI_NULL, VI_NULL, &instr);
        if ( status < VI_SUCCESS)
            throw VisaException(status);

        set_termination_character_state(is_termination_character_needed);
        if (is_termination_character_needed)
            set_termination_character(termination_character);
    }
    virtual void disconnect(){
        auto status = viClose(instr);
        if ( status < VI_SUCCESS)
            throw VisaException(status);
    }
    virtual void clear() {}

    void write(const std::string& msg)
    {
        ViUInt32 writeCount;
        auto status = viWrite (instr, (ViBuf)msg.c_str(), (ViUInt32)msg.size(), &writeCount);
        if ( status < VI_SUCCESS)
            throw VisaException(status);
    }

    std::string read()
    {
        std::vector<unsigned char> data;
        bool data_available=true;
        while(data_available) {
            unsigned char buffer[1024];
            ViUInt32 retCount=0;

            auto status = viRead (instr, buffer, 1024, &retCount);
            data.insert(data.end(), &buffer[0], &buffer[retCount]);

            if ( status < VI_SUCCESS)
                throw VisaException(status);

            /* VI_SUCCESS_MAX_CNT indicates that more data is availvable to read */
            data_available = (status==VI_SUCCESS_MAX_CNT);
        }

        return std::string(reinterpret_cast<const char*>(data.data()), data.size());
    }

    void set_termination_character_state(bool enable)
    {
        int status;
        if (enable==true)
        {
            status = viSetAttribute(instr, VI_ATTR_TERMCHAR_EN, VI_TRUE);
        }
        else
            status = viSetAttribute(instr, VI_ATTR_TERMCHAR_EN, false);
        if ( status < VI_SUCCESS)
            throw VisaException(status);
    }
    bool get_termination_character_state() const
    {
        int result;
        auto status = viGetAttribute(instr, VI_ATTR_TERMCHAR_EN, &result);
        if ( status < VI_SUCCESS)
            throw VisaException(status);

        return result == VI_TRUE;
    }

    void set_termination_character(char term_char)
    {
        auto status = viSetAttribute(instr, VI_ATTR_TERMCHAR, term_char);
        if ( status < VI_SUCCESS)
            throw VisaException(status);
    }

    /* Whether termination characters are needed for this instrument.
     * If true, the termination character is enabled when connnect()
     * is called. */
    bool is_termination_character_needed = true;
    char termination_character = '\n';
  private:
    ViSession resource_manager;
    ViSession instr;
    visa_address_t address;
};

} // namespace udaq::devices
