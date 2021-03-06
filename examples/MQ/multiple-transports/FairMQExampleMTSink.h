/********************************************************************************
 *    Copyright (C) 2014 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    *
 *                                                                              *
 *              This software is distributed under the terms of the             * 
 *         GNU Lesser General Public Licence version 3 (LGPL) version 3,        *  
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/
/**
 * FairMQExampleMTSink.h
 *
 * @since 2014-10-10
 * @author A. Rybalchenko
 */

#ifndef FAIRMQEXAMPLEMTSINK_H_
#define FAIRMQEXAMPLEMTSINK_H_

#include "FairMQDevice.h"

class FairMQExampleMTSink : public FairMQDevice
{
  public:
    FairMQExampleMTSink();
    virtual ~FairMQExampleMTSink();

  protected:
    bool HandleData1(FairMQMessagePtr&, int);
    bool HandleData2(FairMQMessagePtr&, int);
};

#endif /* FAIRMQEXAMPLEMTSINK_H_ */
