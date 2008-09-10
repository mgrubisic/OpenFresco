/* ****************************************************************** **
**    OpenFRESCO - Open Framework                                     **
**                 for Experimental Setup and Control                 **
**                                                                    **
**                                                                    **
** Copyright (c) 2006, The Regents of the University of California    **
** All Rights Reserved.                                               **
**                                                                    **
** Commercial use of this program without express permission of the   **
** University of California, Berkeley, is strictly prohibited. See    **
** file 'COPYRIGHT_UCB' in main directory for information on usage    **
** and redistribution, and for a DISCLAIMER OF ALL WARRANTIES.        **
**                                                                    **
** Developed by:                                                      **
**   Andreas Schellenberg (andreas.schellenberg@gmx.net)              **
**   Yoshikazu Takahashi (yos@catfish.dpri.kyoto-u.ac.jp)             **
**   Gregory L. Fenves (fenves@berkeley.edu)                          **
**   Stephen A. Mahin (mahin@berkeley.edu)                            **
**                                                                    **
** ****************************************************************** */

// $Revision$
// $Date$
// $URL: $

// Written: Andreas Schellenberg (andreas.schellenberg@gmx.net)
// Created: 09/06
// Revision: A
//
// Description: This file contains the implementation of the ECxPCtarget class.

#include "ECxPCtarget.h"

#include <xpcapi.h>
#include <xpcapiconst.h>


ECxPCtarget::ECxPCtarget(int tag, int pctype, char *ipaddress,
    char *ipport, char *appname, char *apppath)
    : ExperimentalControl(tag),
    pcType(pctype), ipAddress(ipaddress), ipPort(ipport),
    appName(appname), appPath(apppath),
    targDisp(0), targVel(0), targAccel(0), measDisp(0), measForce(0),
    measDispId(0), measForceId(0)
{    
    // initialize the xPC Target dynamic link library
    if (xPCInitAPI())  {
        opserr << "ECxPCtarget::ECxPCtarget()"
            << " - xPCInitAPI: unable to load xPC Target API.\n";
        exit(OF_ReturnType_failed);
    }
    
    // the host application OpenSees needs to access the xPC Target;
    // a TCP/IP channel is therefore opened to the xPC Target
    port = xPCOpenTcpIpPort(ipAddress, ipPort);
    if (xPCGetLastError())  {
        xPCErrorMsg(xPCGetLastError(), errMsg);
        opserr << "ECxPCtarget::ECxPCtarget()"
            << " - xPCOpenTcpIpPort: error = " << errMsg << endln;
        xPCFreeAPI();
        exit(OF_ReturnType_failed);
    }
    
    opserr << "****************************************************************\n";
    opserr << "* The TCP/IP channel with address: " << ipAddress << endln;
    opserr << "* and port: " << ipPort << " has been opened\n";
    opserr << "****************************************************************\n";
    opserr << endln;

    // check if target application is already loaded
    // otherwise load the desired application
    char *currentAppName = new char [256];
    xPCGetAppName(port, currentAppName);
    if (xPCGetLastError())  {
        xPCErrorMsg(xPCGetLastError(), errMsg);
        opserr << "ECxPCtarget::ECxPCtarget() - "
            << "xPCGetAppName: error = " << errMsg << endln;
        xPCClosePort(port);
        xPCFreeAPI();
        exit(OF_ReturnType_failed);
    }
    if (strcmp(currentAppName,appName) != 0)  {
        // unload the current application
        xPCUnloadApp(port);
        if (xPCGetLastError())  {
            xPCErrorMsg(xPCGetLastError(), errMsg);
            opserr << "ECxPCtarget::ECxPCtarget() - "
                << "xPCUnloadApp: error = " << errMsg << endln;
            xPCClosePort(port);
            xPCFreeAPI();
            exit(OF_ReturnType_failed);
        }
        // load the new target application
        xPCLoadApp(port, appPath, appName);
        if (xPCGetLastError())  {
            xPCErrorMsg(xPCGetLastError(), errMsg);
            opserr << "ECxPCtarget::ECxPCtarget() - "
                << "xPCLoadApp: error = " << errMsg << endln;
            xPCClosePort(port);
            xPCFreeAPI();
            exit(OF_ReturnType_failed);
        }
    }
    delete [] currentAppName;
    
    // stop the target application on the xPC Target
    xPCStartApp(port);
    if (xPCGetLastError())  {
        xPCErrorMsg(xPCGetLastError(), errMsg);
        opserr << "ECxPCtarget::ECxPCtarget() - "
            << "xPCStartApp: error = " << errMsg << endln;
        xPCClosePort(port);
        xPCFreeAPI();
        exit(OF_ReturnType_failed);
    }
    this->sleep(1000);
    xPCStopApp(port);
    if (xPCGetLastError())  {
        xPCErrorMsg(xPCGetLastError(), errMsg);
        opserr << "ECxPCtarget::ECxPCtarget() - "
            << "xPCStopApp: error = " << errMsg << endln;
        xPCClosePort(port);
        xPCFreeAPI();
        exit(OF_ReturnType_failed);
    }
    
    opserr << "****************************************************************\n";
    opserr << "* The application '" << appName << "' has been loaded and is stopped\n";
    opserr << "* sample time = " << xPCGetSampleTime(port) << ", stop time = " << xPCGetStopTime(port) << endln;
    opserr << "****************************************************************\n";
    opserr << endln;
}


ECxPCtarget::ECxPCtarget(const ECxPCtarget &ec)
    : ExperimentalControl(ec),
    targDisp(0), targVel(0), targAccel(0), measDisp(0), measForce(0),
    measDispId(0), measForceId(0)
{
    pcType = ec.pcType;
    port = ec.port;
    ipAddress = ec.ipAddress;
    ipPort = ec.ipPort;
    appName = ec.appName;
    appPath = ec.appPath;
}


ECxPCtarget::~ECxPCtarget()
{
    // delete memory of target vectors
    if (targDisp != 0)
        delete targDisp;
    if (targVel != 0)
        delete targVel;
    if (targAccel != 0)
        delete targAccel;
    
    // delete memory of measured vectors
    if (measDisp != 0)
        delete measDisp;
    if (measForce != 0)
        delete measForce;
    if (measDispId != 0)
        delete measDispId;
    if (measForceId != 0)
        delete measForceId;
    
    // delete memory of strings
    if (ipAddress != 0)
        delete [] ipAddress;
    if (ipPort != 0)
        delete [] ipPort;
    if (appName != 0)
        delete [] appName;
    if (appPath != 0)
        delete [] appPath;
    
    // stop the target application on the xPC Target
    xPCStopApp(port);
    
    // each application has to close the channel and unregister itself before exiting
    xPCClosePort(port);
    xPCFreeAPI();
    
    opserr << endln;
    opserr << "****************************************\n";
    opserr << "* The rtp application has been stopped *\n";
    opserr << "****************************************\n";
    opserr << endln;
}


int ECxPCtarget::setup()
{
    int rValue = 0;
    
    if (targDisp != 0)
        delete targDisp;
    if (targVel != 0)
        delete targVel;
    if (targAccel != 0)
        delete targAccel;
    
    if ((*sizeCtrl)(OF_Resp_Disp) != 0)  {
        targDisp = new double [(*sizeCtrl)(OF_Resp_Disp)];
        for (int i=0; i<(*sizeCtrl)(OF_Resp_Disp); i++)
            targDisp[i] = 0.0;
    }
    if ((*sizeCtrl)(OF_Resp_Vel) != 0)  {
        targVel = new double [(*sizeCtrl)(OF_Resp_Vel)];
        for (int i=0; i<(*sizeCtrl)(OF_Resp_Vel); i++)
            targVel[i] = 0.0;
    }
    if ((*sizeCtrl)(OF_Resp_Accel) != 0)  {
        targAccel = new double [(*sizeCtrl)(OF_Resp_Accel)];
        for (int i=0; i<(*sizeCtrl)(OF_Resp_Accel); i++)
            targAccel[i] = 0.0;
    }
    
    if (measDisp != 0)
        delete measDisp;
    if (measForce != 0)
        delete measForce;
    
    if ((*sizeDaq)(OF_Resp_Disp) != 0)  {
        measDisp = new double [(*sizeDaq)(OF_Resp_Disp)];
        measDispId = new int [(*sizeDaq)(OF_Resp_Disp)];
        for (int i=0; i<(*sizeDaq)(OF_Resp_Disp); i++)  {
            measDisp[i] = 0.0;
            measDispId[i] = 0;
        }
    }
    if ((*sizeDaq)(OF_Resp_Force) != 0)  {
        measForce = new double [(*sizeDaq)(OF_Resp_Force)];
        measForceId = new int [(*sizeDaq)(OF_Resp_Force)];
        for (int i=0; i<(*sizeDaq)(OF_Resp_Force); i++)  {
            measForce[i] = 0.0;
            measForceId[i] = 0;
        }
    }
    
    // get addresses of the controlled variables on the xPC Target
    newTargetId = xPCGetParamIdx(port, "xPC HC/newTarget", "Value");
    if (xPCGetLastError())  {
        xPCErrorMsg(xPCGetLastError(), errMsg);
        opserr << "ECxPCtarget::setup() - "
            << "xPCGetParamIdx - newTarget: error = " << errMsg << endln;
        xPCClosePort(port);
        xPCFreeAPI();
        exit(OF_ReturnType_failed);
    }
    switchPCId = xPCGetSignalIdx(port, "xPC HC/switchPC");
    if (xPCGetLastError())  {
        xPCErrorMsg(xPCGetLastError(), errMsg);
        opserr << "ECxPCtarget::setup() - "
            << "xPCGetSignalIdx - switchPC: error = " << errMsg << endln;
        xPCClosePort(port);
        xPCFreeAPI();
        exit(OF_ReturnType_failed);
    }
    atTargetId = xPCGetSignalIdx(port, "xPC HC/atTarget");
    if (xPCGetLastError())  {
        xPCErrorMsg(xPCGetLastError(), errMsg);
        opserr << "ECxPCtarget::setup() - "
            << "xPCGetSignalIdx - atTarget: error = " << errMsg << endln;
        xPCClosePort(port);
        xPCFreeAPI();
        exit(OF_ReturnType_failed);
    }
    targDispId = xPCGetParamIdx(port, "xPC HC/targDsp", "Value");
    if (xPCGetLastError())  {
        xPCErrorMsg(xPCGetLastError(), errMsg);
        opserr << "ECxPCtarget::setup() - "
            << "xPCGetParamIdx - targDsp: error = " << errMsg << endln;
        xPCClosePort(port);
        xPCFreeAPI();
        exit(OF_ReturnType_failed);
    }
    if (pcType==2 || pcType==3)  {
        targVelId = xPCGetParamIdx(port, "xPC HC/targVel", "Value");
        if (xPCGetLastError())  {
            xPCErrorMsg(xPCGetLastError(), errMsg);
            opserr << "ECxPCtarget::setup() - "
                << "xPCGetParamIdx - targVel: error = " << errMsg << endln;
            xPCClosePort(port);
            xPCFreeAPI();
            exit(OF_ReturnType_failed);
        }
    }
    if (pcType==3)  {
        targAccelId = xPCGetParamIdx(port, "xPC HC/targAcc", "Value");
        if (xPCGetLastError())  {
            xPCErrorMsg(xPCGetLastError(), errMsg);
            opserr << "ECxPCtarget::setup() - "
                << "xPCGetParamIdx - targAcc: error = " << errMsg << endln;
            xPCClosePort(port);
            xPCFreeAPI();
            exit(OF_ReturnType_failed);
        }
    }
    if ((*sizeDaq)(OF_Resp_Disp)==1)  {
        measDispId[0] = xPCGetSignalIdx(port, "xPC HC/measDsp");
        if (xPCGetLastError())  {
            xPCErrorMsg(xPCGetLastError(), errMsg);
            opserr << "ECxPCtarget::setup() - "
                << "xPCGetSignalIdx - measDsp: error = " << errMsg << endln;
            xPCClosePort(port);
            xPCFreeAPI();
            exit(OF_ReturnType_failed);
        }
    } else  {
        char dspStr[20], sigStr[3];
        for (int i=0; i<(*sizeDaq)(OF_Resp_Disp); i++)  {
            strcpy(dspStr,"xPC HC/measDsp/s");
            sprintf(sigStr, "%d", i+1);
            strcat(dspStr, sigStr);
            measDispId[i] = xPCGetSignalIdx(port, dspStr);
            if (xPCGetLastError())  {
                xPCErrorMsg(xPCGetLastError(), errMsg);
                opserr << "ECxPCtarget::setup() - "
                    << "xPCGetSignalIdx - measDsp: error = " << errMsg << endln;
                xPCClosePort(port);
                xPCFreeAPI();
                exit(OF_ReturnType_failed);
            }
        }
    }
    if ((*sizeDaq)(OF_Resp_Force)==1)  {
        measForceId[0] = xPCGetSignalIdx(port, "xPC HC/measFrc");
        if (xPCGetLastError())  {
            xPCErrorMsg(xPCGetLastError(), errMsg);
            opserr << "ECxPCtarget::setup() - xPCGetSignalIdx - measFrc: error = " << errMsg << endln;
            xPCClosePort(port);
            xPCFreeAPI();
            exit(OF_ReturnType_failed);
        }
    } else  {
        char frcStr[20], sigStr[3];
        for (int i=0; i<(*sizeDaq)(OF_Resp_Force); i++)  {
            strcpy(frcStr,"xPC HC/measFrc/s");
            sprintf(sigStr, "%d", i+1);
            strcat(frcStr, sigStr);
            measForceId[i] = xPCGetSignalIdx(port, frcStr);
            if (xPCGetLastError())  {
                xPCErrorMsg(xPCGetLastError(), errMsg);
                opserr << "ECxPCtarget::setup() - "
                    << "xPCGetSignalIdx - measFrc: error = " << errMsg << endln;
                xPCClosePort(port);
                xPCFreeAPI();
                exit(OF_ReturnType_failed);
            }
        }
    }
    
    // print experimental control information
    this->Print(opserr);
    
    opserr << "****************************************************************\n";
    opserr << "* Make sure that offset values of controller are set to ZERO   *\n";
    opserr << "*                                                              *\n";
    opserr << "* Press 'Enter' to proceed or 'c' to cancel the initialization *\n";
    opserr << "****************************************************************\n";
    opserr << endln;
    int c = getchar();
    if (c == 'c')  {
        getchar();
        xPCClosePort(port);
        xPCFreeAPI();
        exit(OF_ReturnType_failed);
    }
    
    // start the target application on the xPC Target
    xPCStartApp(port);
    if (xPCGetLastError())  {
        xPCErrorMsg(xPCGetLastError(), errMsg);
        opserr << "ECxPCtarget::setup() - "
            << "xPCStartApp: error = " << errMsg << endln;
        xPCClosePort(port);
        xPCFreeAPI();
        exit(OF_ReturnType_failed);
    }
    this->sleep(1000);
		
	do  {
        rValue += this->control();
        rValue += this->acquire();
        
        int i;
        opserr << "****************************************************************\n";
        opserr << "* Initial values of DAQ are:\n";
        opserr << "*\n";
        opserr << "* dspDaq = [";
        for (i=0; i<(*sizeDaq)(OF_Resp_Disp); i++)
            opserr << " " << measDisp[i];
        opserr << " ]\n";
        opserr << "* frcDaq = [";
        for (i=0; i<(*sizeDaq)(OF_Resp_Force); i++)
            opserr << " " << measForce[i];
        opserr << " ]\n";
        opserr << "*\n";
        opserr << "* Press 'Enter' to start the test or\n";
        opserr << "* 'r' to repeat the measurement or\n";
        opserr << "* 'c' to cancel the initialization\n";
        opserr << "****************************************************************\n";
        opserr << endln;
        c = getchar();
        if (c == 'c')  {
            getchar();
            xPCClosePort(port);
            xPCFreeAPI();
            exit(OF_ReturnType_failed);
        } else if (c == 'r')  {
            getchar();
        }
    } while (c == 'r');
    
    opserr << "*****************\n";
    opserr << "* Running...... *\n";
    opserr << "*****************\n";
    opserr << endln;
    
    return rValue;
}


int ECxPCtarget::setSize(ID sizeT, ID sizeO)
{
    // check sizeTrial and sizeOut
    // for ECxPCtarget object
    
    // ECxPCtarget objects only use 
    // disp or disp and vel or disp, vel and accel for trial and
    // disp and force for output
    // check these are available in sizeT/sizeO.
    if ((pcType == 0 && sizeT(OF_Resp_Disp) == 0) || 
        (pcType == 1 && sizeT(OF_Resp_Disp) == 0 && sizeT(OF_Resp_Vel) == 0) ||
        (pcType == 2 && sizeT(OF_Resp_Disp) == 0 && sizeT(OF_Resp_Vel) == 0 && sizeT(OF_Resp_Accel) == 0) ||
        (sizeO(OF_Resp_Disp) == 0 && sizeO(OF_Resp_Force) == 0))  {
        opserr << "ECxPCtarget::setSize() - wrong sizeTrial/Out\n"; 
        opserr << "see User Manual.\n";
        xPCClosePort(port);
        xPCFreeAPI();
        exit(OF_ReturnType_failed);
    }
    
    *sizeCtrl = sizeT;
    *sizeDaq  = sizeO;

    return OF_ReturnType_completed;
}


int ECxPCtarget::setTrialResponse(const Vector* disp,
    const Vector* vel,
    const Vector* accel,
    const Vector* force,
    const Vector* time)
{
    int i, rValue = 0;
    if (disp != 0)  {
        for (i=0; i<(*sizeCtrl)(OF_Resp_Disp); i++)  {
            targDisp[i] = (*disp)(i);
            if (theCtrlFilters[OF_Resp_Disp] != 0)
                targDisp[i] = theCtrlFilters[OF_Resp_Disp]->filtering(targDisp[i]);
        }
    }
    if (vel != 0)  {
        for (i=0; i<(*sizeCtrl)(OF_Resp_Vel); i++)  {
            targVel[i] = (*vel)(i);
            if (theCtrlFilters[OF_Resp_Vel] != 0)
                targVel[i] = theCtrlFilters[OF_Resp_Vel]->filtering(targVel[i]);
        }
    }
    if (accel != 0)  {
        for (i=0; i<(*sizeCtrl)(OF_Resp_Accel); i++)  {
            targAccel[i] = (*accel)(i);
            if (theCtrlFilters[OF_Resp_Accel] != 0)
                targAccel[i] = theCtrlFilters[OF_Resp_Accel]->filtering(targAccel[i]);
        }
    }
    
    rValue = this->control();
    
    return rValue;
}


int ECxPCtarget::getDaqResponse(Vector* disp,
    Vector* vel,
    Vector* accel,
    Vector* force,
    Vector* time)
{
    this->acquire();
    
    int i;
    if (disp != 0)  {
        for (i=0; i<(*sizeDaq)(OF_Resp_Disp); i++)  {
            if (theDaqFilters[OF_Resp_Disp] != 0)
                measDisp[i] = theDaqFilters[OF_Resp_Disp]->filtering(measDisp[i]);
            (*disp)(i) = measDisp[i];
        }
    }
    if (force != 0)  {
        for (i=0; i<(*sizeDaq)(OF_Resp_Force); i++)  {
            if (theDaqFilters[OF_Resp_Force] != 0)
                measForce[i] = theDaqFilters[OF_Resp_Force]->filtering(measForce[i]);
            (*force)(i) = measForce[i];
        }
    }
        
    return OF_ReturnType_completed;
}


int ECxPCtarget::commitState()
{	
    return OF_ReturnType_completed;
}


ExperimentalControl *ECxPCtarget::getCopy()
{
    return new ECxPCtarget(*this);
}


Response* ECxPCtarget::setResponse(const char **argv, int argc,
    OPS_Stream &output)
{
    int i;
    char outputData[15];
    Response *theResponse = 0;
    
    output.tag("ExpControlOutput");
    output.attr("ctrlType",this->getClassType());
    output.attr("ctrlTag",this->getTag());
        
    // target displacements
    if (strcmp(argv[0],"targDisp") == 0 ||
        strcmp(argv[0],"targetDisp") == 0 ||
        strcmp(argv[0],"targetDisplacement") == 0 ||
        strcmp(argv[0],"targetDisplacements") == 0)
    {
        for (i=0; i<(*sizeCtrl)(OF_Resp_Disp); i++)  {
            sprintf(outputData,"targDisp%d",i+1);
            output.tag("ResponseType",outputData);
        }
        theResponse = new ExpControlResponse(this, 1,
            Vector((*sizeCtrl)(OF_Resp_Disp)));
    }
    
    // target velocities
    if (strcmp(argv[0],"targVel") == 0 ||
        strcmp(argv[0],"targetVel") == 0 ||
        strcmp(argv[0],"targetVelocity") == 0 ||
        strcmp(argv[0],"targetVelocities") == 0)
    {
        for (i=0; i<(*sizeCtrl)(OF_Resp_Vel); i++)  {
            sprintf(outputData,"targVel%d",i+1);
            output.tag("ResponseType",outputData);
        }
        theResponse = new ExpControlResponse(this, 2,
            Vector((*sizeCtrl)(OF_Resp_Vel)));
    }
    
    // target accelerations
    if (strcmp(argv[0],"targAccel") == 0 ||
        strcmp(argv[0],"targetAccel") == 0 ||
        strcmp(argv[0],"targetAcceleration") == 0 ||
        strcmp(argv[0],"targetAccelerations") == 0)
    {
        for (i=0; i<(*sizeCtrl)(OF_Resp_Accel); i++)  {
            sprintf(outputData,"targAccel%d",i+1);
            output.tag("ResponseType",outputData);
        }
        theResponse = new ExpControlResponse(this, 3,
            Vector((*sizeCtrl)(OF_Resp_Accel)));
    }
    
    // measured displacements
    if (strcmp(argv[0],"measDisp") == 0 ||
        strcmp(argv[0],"measuredDisp") == 0 ||
        strcmp(argv[0],"measuredDisplacement") == 0 ||
        strcmp(argv[0],"measuredDisplacements") == 0)
    {
        for (i=0; i<(*sizeDaq)(OF_Resp_Disp); i++)  {
            sprintf(outputData,"measDisp%d",i+1);
            output.tag("ResponseType",outputData);
        }
        theResponse = new ExpControlResponse(this, 4,
            Vector((*sizeDaq)(OF_Resp_Disp)));
    }
    
    // measured forces
    if (strcmp(argv[0],"measForce") == 0 ||
        strcmp(argv[0],"measuredForce") == 0 ||
        strcmp(argv[0],"measuredForces") == 0)
    {
        for (i=0; i<(*sizeDaq)(OF_Resp_Force); i++)  {
            sprintf(outputData,"measForce%d",i+1);
            output.tag("ResponseType",outputData);
        }
        theResponse = new ExpControlResponse(this, 5,
            Vector((*sizeDaq)(OF_Resp_Force)));
    }
    
    output.endTag();
    
    return theResponse;
}


int ECxPCtarget::getResponse(int responseID, Information &info)
{
    Vector resp(0);

    switch (responseID)  {
    case 1:  // target displacements
        resp.setData(targDisp,(*sizeCtrl)(OF_Resp_Disp));
        return info.setVector(resp);
        
    case 2:  // target velocities
        resp.setData(targVel,(*sizeCtrl)(OF_Resp_Vel));
        return info.setVector(resp);
        
    case 3:  // target accelerations
        resp.setData(targAccel,(*sizeCtrl)(OF_Resp_Accel));
        return info.setVector(resp);
        
    case 4:  // measured displacements
        resp.setData(measDisp,(*sizeDaq)(OF_Resp_Disp));
        return info.setVector(resp);
        
    case 5:  // measured forces
        resp.setData(measForce,(*sizeDaq)(OF_Resp_Force));
        return info.setVector(resp);
        
    default:
        return -1;
    }
}


void ECxPCtarget::Print(OPS_Stream &s, int flag)
{
    s << "****************************************************************\n";
    s << "* ExperimentalControl: " << this->getTag() << endln; 
    s << "*   type: ECxPCtarget\n";
    s << "*   ipAddress: " << ipAddress << ", ipPort: " << ipPort << endln;
    s << "*   appName: " << appName << endln;
    s << "*   appPath: " << appPath << endln;
    s << "*   pcType: " << pcType << endln;
    s << "*   ctrlFilters:";
    for (int i=0; i<OF_Resp_All; i++)  {
        if (theCtrlFilters[i] != 0)
            s << " " << theCtrlFilters[i]->getTag();
        else
            s << " 0";
    }
    s << "\n*   daqFilters:";
    for (int i=0; i<OF_Resp_All; i++)  {
        if (theCtrlFilters[i] != 0)
            s << " " << theCtrlFilters[i]->getTag();
        else
            s << " 0";
    }
    s << endln;
    s << "****************************************************************\n";
    s << endln;
}


int ECxPCtarget::control()
{
    // send targDisp, targVel and targAccel and set newTarget flag
    xPCSetParam(port, targDispId, targDisp);
    if (xPCGetLastError())  {
        xPCErrorMsg(xPCGetLastError(), errMsg);
        opserr << "ECxPCtarget::control() - "
            << "xPCSetParam(targDisp): error = " << errMsg << endln;
        xPCClosePort(port);
        xPCFreeAPI();
        exit(OF_ReturnType_failed);
    }
    if (pcType==2 || pcType==3)  {
        xPCSetParam(port, targVelId, targVel);
        if (xPCGetLastError())  {
            xPCErrorMsg(xPCGetLastError(), errMsg);
            opserr << "ECxPCtarget::control() - "
                << "xPCSetParam(targVel): error = " << errMsg << endln;
            xPCClosePort(port);
            xPCFreeAPI();
            exit(OF_ReturnType_failed);
        }
    }
    if (pcType==3)  {
        xPCSetParam(port, targAccelId, targAccel);
        if (xPCGetLastError())  {
            xPCErrorMsg(xPCGetLastError(), errMsg);
            opserr << "ECxPCtarget::control() - "
                << "xPCSetParam(targAccel): error = " << errMsg << endln;
            xPCClosePort(port);
            xPCFreeAPI();
            exit(OF_ReturnType_failed);
        }
    }
    
	// set newTarget flag
	newTarget = 1;
	xPCSetParam(port, newTargetId, &newTarget);
    if (xPCGetLastError())  {
        xPCErrorMsg(xPCGetLastError(), errMsg);
        opserr << "ECxPCtarget::control() - "
            << "xPCSetParam(newTarget): error = " << errMsg << endln;
        xPCClosePort(port);
        xPCFreeAPI();
        exit(OF_ReturnType_failed);
    }
    
	// wait until switchPC flag has changed as well
    switchPC = 0;
    while (switchPC != 1)  {
        switchPC = xPCGetSignal(port, switchPCId);
        if (xPCGetLastError())  {
            xPCErrorMsg(xPCGetLastError(), errMsg);
            opserr << "ECxPCtarget::control() - "
                << "xPCGetSignal(switchPC): error = " << errMsg << endln;
            xPCClosePort(port);
            xPCFreeAPI();
            exit(OF_ReturnType_failed);
        }
    }
    
    // reset newTarget flag
    newTarget = 0;
    xPCSetParam(port, newTargetId, &newTarget);
    if (xPCGetLastError())  {
        xPCErrorMsg(xPCGetLastError(), errMsg);
        opserr << "ECxPCtarget::control() - "
            << "xPCSetParam(newTarget): error = " << errMsg << endln;
        xPCClosePort(port);
        xPCFreeAPI();
        exit(OF_ReturnType_failed);
    }
    
	// wait until switchPC flag has changed as well
    switchPC = 1;
	while (switchPC != 0)  {
        switchPC = xPCGetSignal(port, switchPCId);
        if (xPCGetLastError())  {
            xPCErrorMsg(xPCGetLastError(), errMsg);
            opserr << "ECxPCtarget::control() - "
                << "xPCGetSignal(switchPC): error = " << errMsg << endln;
            xPCClosePort(port);
            xPCFreeAPI();
            exit(OF_ReturnType_failed);
        }
    }
    
    return OF_ReturnType_completed;
}


int ECxPCtarget::acquire()
{
    // wait until target is reached
    atTarget = 0;
    while (atTarget != 1)  {
        atTarget = xPCGetSignal(port, atTargetId);
        if (xPCGetLastError())  {
            xPCErrorMsg(xPCGetLastError(), errMsg);
            opserr << "ECxPCtarget::acquire() - "
                << "xPCGetSignal(atTarget): error = " << errMsg << endln;
            xPCClosePort(port);
            xPCFreeAPI();
            exit(OF_ReturnType_failed);
        }
    }
    
    // read displacements and resisting forces at target
    if ((*sizeDaq)(OF_Resp_Disp) != 0)  {
        xPCGetSignals(port, (*sizeDaq)(OF_Resp_Disp), measDispId, measDisp);
        if (xPCGetLastError())  {
            xPCErrorMsg(xPCGetLastError(), errMsg);
            opserr << "ECxPCtarget::acquire() - "
                << "xPCGetSignal(measDisp): error = " << errMsg << endln;
            xPCClosePort(port);
            xPCFreeAPI();
            exit(OF_ReturnType_failed);
        }
    }
    if ((*sizeDaq)(OF_Resp_Force) != 0)  {
        xPCGetSignals(port, (*sizeDaq)(OF_Resp_Force), measForceId, measForce);
        if (xPCGetLastError())  {
            xPCErrorMsg(xPCGetLastError(), errMsg);
            opserr << "ECxPCtarget::acquire() - "
                << "xPCGetSignal(measForce): error = " << errMsg << endln;
            xPCClosePort(port);
            xPCFreeAPI();
            exit(OF_ReturnType_failed);
        }
    }
    
    return OF_ReturnType_completed;
}


void ECxPCtarget::sleep(const clock_t wait)
{
    clock_t goal;
    goal = wait + clock();
    while (goal>clock());
}
