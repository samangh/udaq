/********************************************************************/
/*              Read and Write to an Instrument Example             */
/*                                                                  */
/* This code demonstrates synchronous read and write commands to a  */
/* GPIB, serial or message-based VXI instrument using VISA.         */
/*                                                                  */
/* The general flow of the code is                                  */
/*      Open Resource Manager                                       */
/*      Open VISA Session to an Instrument                          */
/*      Write the Identification Query Using viWrite                */
/*      Try to Read a Response With viRead                          */
/*      Close the VISA Session                                      */
/********************************************************************/

#include <iostream>
#include <string>

#include <visa.h>

static ViSession defaultRM;
static ViSession instr;
static ViStatus status;
static ViUInt32 retCount;
static ViUInt32 writeCount;
static unsigned char buffer[100];

const std::string query = "*IDN?";

int main(void)
{
    status=viOpenDefaultRM (&defaultRM);
    if (status < VI_SUCCESS)
    {
        std::cout << "Could not open a session to the VISA Resource Manager!" <<std::endl;
        exit (EXIT_FAILURE);
    }

    status = viOpen (defaultRM,  "TCPIP0::127.0.0.1::5020::SOCKET", VI_NULL, VI_NULL, &instr);
    if (status < VI_SUCCESS)
    {
        std::cout << "Cannot open a session to the device." << std::endl;
        goto Close;
    }

    // Set time outs, and enable the use of a terminatin character (default is \n)
    viSetAttribute (instr, VI_ATTR_TMO_VALUE, 5000);
    viSetAttribute(instr, VI_ATTR_TERMCHAR_EN, VI_TRUE);

    status = viWrite (instr, (ViBuf)query.c_str(), (ViUInt32)sizeof(query.c_str()), &writeCount);
    if (status < VI_SUCCESS)
    {
        std::cout << "Error writing to the device" << std::endl;
        goto Close;
    }

    // Read up to 1024 bytes, or until the end-character is received
    status = viRead (instr, buffer, 1024, &retCount);
    if (status < VI_SUCCESS)
        std::cerr << "Error reading a response from the device" << std::endl;
    else
        std::cout << "Data read: " << buffer << std::endl;

Close:
    std::cout << "Closing Sessions\nHit enter to continue";
    fflush(stdin);
    getchar();
    status = viClose(instr);
    status = viClose(defaultRM);

    return 0;
}
