#pragma once

#include <vector>

#include <visa.h>

#include <udaq/types.h>
#include "visa_exception.h"
#include "conn_options.h"

namespace udaq::visa {

class visa_comm_driver {
  public:
    visa_comm_driver() {
        auto status= viOpenDefaultRM (&m_resource_manager);
        if ( status < VI_SUCCESS)
            throw udaq::visa::VisaException(status);
    }
    ~visa_comm_driver() {
        viClose(m_resource_manager);
    }

    /* Connects to the instrument. */
    virtual void connect(ConnectionParameters conn_param)
    {
        m_conn_parameters = conn_param;

        auto status = viOpen (m_resource_manager,  conn_param.address.c_str(), VI_NULL, VI_NULL, &m_session);
        if ( status < VI_SUCCESS)
            throw VisaException(status);

        m_connection_established = true;

        set_termination_character_state(conn_param.termination_character_needed);
        if (conn_param.termination_character_needed)
            set_termination_character(conn_param.termination_character);
    }
    virtual void disconnect(){
        auto status = viClose(m_session);
        if ( status < VI_SUCCESS)
            throw VisaException(status);

        m_connection_established = false;
    }
    virtual bool is_connected() {
        if (!m_connection_established)
            return false;
        try {
            get_termination_character_state();
        }  catch (...) {
            return false;
        }

        return true;

    }
    virtual void clear()
    {
        auto status = viClear(m_session);
        if ( status < VI_SUCCESS)
            throw VisaException(status);
    }

    void write(const std::string& msg)
    {
        ViUInt32 writeCount;
        auto status = viWrite (m_session, (ViBuf)msg.c_str(), (ViUInt32)msg.size(), &writeCount);
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

            auto status = viRead (m_session, buffer, 1024, &retCount);
            data.insert(data.end(), &buffer[0], &buffer[retCount]);

            if ( status < VI_SUCCESS)
                throw VisaException(status);

            /* VI_SUCCESS_MAX_CNT indicates that more data is availvable to read */
            data_available = (status==VI_SUCCESS_MAX_CNT);
        }

        return std::string(reinterpret_cast<const char*>(data.data()), data.size());
    }

    void set_termination_character_state(bool state)
    {
        auto status = viSetAttribute(m_session, VI_ATTR_TERMCHAR_EN, state);
        if ( status < VI_SUCCESS)
            throw VisaException(status);
    }
    bool get_termination_character_state()
    {
        int result;
        auto status = viGetAttribute(m_session, VI_ATTR_TERMCHAR_EN, &result);
        if ( status < VI_SUCCESS)
            throw VisaException(status);

        return result == VI_TRUE;
    }
    void set_termination_character(char term_char)
    {
        auto status = viSetAttribute(m_session, VI_ATTR_TERMCHAR, term_char);
        if ( status < VI_SUCCESS)
            throw VisaException(status);
    }
    ConnectionParameters get_connection_parameters(){
        return m_conn_parameters;
    }
  private:
    ViSession m_resource_manager;
    ViSession m_session;
    ConnectionParameters m_conn_parameters;
    bool m_connection_established =false;
};

} // namespace udaq::devices
