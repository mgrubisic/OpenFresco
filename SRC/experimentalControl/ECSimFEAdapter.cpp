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

// $Revision: $
// $Date: $
// $URL: $

// Written: Andreas Schellenberg (andreas.schellenberg@gmx.net)
// Created: 09/07
// Revision: A
//
// Description: This file contains the implementation of the
// ECSimFEAdapter class.

#include "ECSimFEAdapter.h"

#include <Channel.h>
#include <TCP_Socket.h>


ECSimFEAdapter::ECSimFEAdapter(int tag,    
    char *ipaddress, int ipport)
    : ECSimulation(tag),
    ipAddress(ipaddress), ipPort(ipport),
    dataSize(OF_Network_dataSize), theChannel(0),
    sData(0), sendData(0), rData(0), recvData(0),
    targDisp(0), targForce(0), measDisp(0), measForce(0)
{   
    // setup the connection
    theChannel = new TCP_Socket(ipPort, ipAddress);
    if (theChannel->setUpConnection() != 0)  {
        opserr << "ECSimFEAdapter::ECSimFEAdapter() - "
            << "failed to setup TCP connection to adapter element.\n";
        delete theChannel;
        exit(OF_ReturnType_failed);
    }
    
    opserr << "****************************************************************\n";
    opserr << "* The channel with address: " << ipAddress << endln;
    opserr << "* and port: " << ipPort << " has been opened\n";
    opserr << "****************************************************************\n";
    opserr << endln;
    
    // allocate memory for the send vectors
    sData = new double [dataSize];
    sendData = new Vector(sData, dataSize);
    sendData->Zero();
    
    // allocate memory for the receive vectors
    rData = new double [dataSize];
    recvData = new Vector(rData, dataSize);
    recvData->Zero();
}


ECSimFEAdapter::ECSimFEAdapter(const ECSimFEAdapter &ec)
    : ECSimulation(ec),
    dataSize(OF_Network_dataSize), theChannel(0),
    sData(0), sendData(0), rData(0), recvData(0),
    targDisp(0), targForce(0), measDisp(0), measForce(0)
{
    // use the existing channel which is set up
    ipAddress = ec.ipAddress;
    ipPort = ec.ipPort;
    theChannel = ec.theChannel;

    // allocate memory for the send vectors
    sData = new double [dataSize];
    sendData = new Vector(sData, dataSize);
    sendData->Zero();

    // allocate memory for the receive vectors
    rData = new double [dataSize];
    recvData = new Vector(rData, dataSize);
    recvData->Zero();
}


ECSimFEAdapter::~ECSimFEAdapter()
{
    // send termination to adapter element
    sData[0] = OF_RemoteTest_DIE;
    theChannel->sendVector(0, 0, *sendData, 0);

    // delete memory of target vectors
    if (targDisp != 0)
        delete targDisp;
    if (targForce != 0)
        delete targForce;
    
    // delete memory of measured vectors
    if (measDisp != 0)
        delete measDisp;
    if (measForce != 0)
        delete measForce;

    // delete memory of string
    if (ipAddress != 0)
        delete [] ipAddress;
    
    // close connection by destroying theChannel
    if (sendData != 0)
        delete sendData;
    if (sData != 0)
        delete [] sData;
    if (recvData != 0)
        delete recvData;
    if (rData != 0)
        delete [] rData;
    if (theChannel != 0)
        delete theChannel;

    opserr << endln;
    opserr << "***********************************************************\n";
    opserr << "* The connection with the adapter element has been closed *\n";
    opserr << "***********************************************************\n";
    opserr << endln;
}


int ECSimFEAdapter::setup()
{
    if (targDisp != 0)
        delete targDisp;
    if (targForce != 0)
        delete targForce;
    
    int id = 1;
    if ((*sizeCtrl)(OF_Resp_Disp) != 0)  {
        targDisp = new Vector(&sData[id],(*sizeCtrl)(OF_Resp_Disp));
        id += (*sizeCtrl)(OF_Resp_Disp);
    }
    if ((*sizeCtrl)(OF_Resp_Force) != 0)  {
        targForce = new Vector(&sData[id],(*sizeCtrl)(OF_Resp_Force));
        id += (*sizeCtrl)(OF_Resp_Force);
    }
    
    if (measDisp != 0)
        delete measDisp;
    if (measForce != 0)
        delete measForce;
    
    id = 0;
    if ((*sizeDaq)(OF_Resp_Disp) != 0)  {
        measDisp = new Vector(&rData[id],(*sizeDaq)(OF_Resp_Disp));
        id += (*sizeDaq)(OF_Resp_Disp);
    }
    if ((*sizeDaq)(OF_Resp_Force) != 0)  {
        measForce = new Vector(&rData[id],(*sizeDaq)(OF_Resp_Force));
        id += (*sizeDaq)(OF_Resp_Force);
    }
    
    // send the data size to the adapter element
    ID idData(2*OF_Resp_All+1);
    for (int i=0; i<OF_Resp_All; i++)  {
        idData(i) = (*sizeCtrl)(i);
        idData(OF_Resp_All+i) = (*sizeDaq)(i);
    }
    idData(2*OF_Resp_All) = dataSize;
    theChannel->sendID(0, 0, idData, 0);

    // print experimental control information
    this->Print(opserr);
    
    opserr << "*****************\n";
    opserr << "* Running...... *\n";
    opserr << "*****************\n";
    opserr << endln;
    
    return OF_ReturnType_completed;
}


int ECSimFEAdapter::setSize(ID sizeT, ID sizeO)
{
    // check sizeTrial and sizeOut
    // for ECSimFEAdapter object
    
    // ECSimFEAdapter objects can only use 
    // disp and force for trial and disp and force for output
    // check these are available in sizeT/sizeO.
    if ((sizeT(OF_Resp_Disp) == 0 && sizeT(OF_Resp_Force) == 0) ||
        (sizeO(OF_Resp_Disp) == 0) || (sizeO(OF_Resp_Force) == 0))  {
        opserr << "ECSimFEAdapter::setSize() - wrong sizeTrial/Out\n"; 
        opserr << "see User Manual.\n";
        delete theChannel;
        exit(OF_ReturnType_failed);
    }
    
    *sizeCtrl = sizeT;
    *sizeDaq  = sizeO;
    
    return OF_ReturnType_completed;
}


int ECSimFEAdapter::setTrialResponse(const Vector* disp,
    const Vector* vel,
    const Vector* accel,
    const Vector* force,
    const Vector* time)
{
    int i, rValue = 0;
    if (disp != 0)  {
        *targDisp = *disp;
        if (theCtrlFilters[OF_Resp_Disp] != 0)  {
            for (i=0; i<(*sizeCtrl)(OF_Resp_Disp); i++)
                (*targDisp)(i) = theCtrlFilters[OF_Resp_Disp]->filtering((*targDisp)(i));
        }
    }
    if (force != 0)  {
        *targForce = *force;
        if (theCtrlFilters[OF_Resp_Force] != 0)  {
            for (i=0; i<(*sizeCtrl)(OF_Resp_Force); i++)
                (*targForce)(i) = theCtrlFilters[OF_Resp_Force]->filtering((*targForce)(i));
        }
    }

    rValue = this->control();
    
    return rValue;
}


int ECSimFEAdapter::getDaqResponse(Vector* disp,
    Vector* vel,
    Vector* accel,
    Vector* force,
    Vector* time)
{
    this->acquire();
    
    int i;
    if (disp != 0)  {
        if (theDaqFilters[OF_Resp_Disp] != 0)  {
            for (i=0; i<(*sizeDaq)(OF_Resp_Disp); i++)
                (*measDisp)(i) = theDaqFilters[OF_Resp_Disp]->filtering((*measDisp)(i));
        }
        *disp = *measDisp;
    }
    if (force != 0)  {
        if (theDaqFilters[OF_Resp_Force] != 0)  {
            for (i=0; i<(*sizeDaq)(OF_Resp_Force); i++)
                (*measForce)(i) = theDaqFilters[OF_Resp_Force]->filtering((*measForce)(i));
        }
        *force = *measForce;
    }
        
    return OF_ReturnType_completed;
}


int ECSimFEAdapter::commitState()
{	
    return OF_ReturnType_completed;
}


ExperimentalControl *ECSimFEAdapter::getCopy()
{
    return new ECSimFEAdapter(*this);
}


Response* ECSimFEAdapter::setResponse(const char **argv, int argc,
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
    
    // target forces
    if (strcmp(argv[0],"targForce") == 0 ||
        strcmp(argv[0],"targetForce") == 0 ||
        strcmp(argv[0],"targetForces") == 0)
    {
        for (i=0; i<(*sizeCtrl)(OF_Resp_Force); i++)  {
            sprintf(outputData,"targForce%d",i+1);
            output.tag("ResponseType",outputData);
        }
        theResponse = new ExpControlResponse(this, 2,
            Vector((*sizeCtrl)(OF_Resp_Force)));
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
        theResponse = new ExpControlResponse(this, 3,
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
        theResponse = new ExpControlResponse(this, 4,
            Vector((*sizeDaq)(OF_Resp_Force)));
    }
    
    output.endTag();
    
    return theResponse;
}


int ECSimFEAdapter::getResponse(int responseID, Information &info)
{
    switch (responseID)  {
    case 1:  // target displacements
        return info.setVector(*targDisp);
        
    case 2:  // target forces
        return info.setVector(*targForce);
        
    case 3:  // measured displacements
        return info.setVector(*measDisp);
        
    case 4:  // measured forces
        return info.setVector(*measForce);
        
    default:
        return -1;
    }
}


void ECSimFEAdapter::Print(OPS_Stream &s, int flag)
{
    s << "****************************************************************\n";
    s << "* ExperimentalControl: " << this->getTag() << endln; 
    s << "*   type: ECSimFEAdapter\n";
    s << "*   ipAddress: " << ipAddress << ", ipPort: " << ipPort << endln;
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


int ECSimFEAdapter::control()
{
    sData[0] = OF_RemoteTest_setTrialResponse;
    theChannel->sendVector(0, 0, *sendData, 0);

    return OF_ReturnType_completed;
}


int ECSimFEAdapter::acquire()
{
    sData[0] = OF_RemoteTest_getForce;
    theChannel->sendVector(0, 0, *sendData, 0);
    theChannel->recvVector(0, 0, *recvData, 0);

    return OF_ReturnType_completed;
}


void ECSimFEAdapter::sleep(const clock_t wait)
{
    clock_t goal;
    goal = wait + clock();
    while (goal>clock());
}
