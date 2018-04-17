// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _ISBRIDGE_H_
#define _ISBRIDGE_H_

class ISSubscriber;

/** Base class for publishers. Must know how to write into the destination protocol */
class ISPublisher
{
public:
    virtual void onTerminate() {}
    virtual ~ISPublisher() = default;
    virtual bool publish(void *data) = 0;
};

/** Base class for subscribers. Must know how to read from the origin protocol */
class ISSubscriber
{
protected:
    ISPublisher *listener_publisher;
public:
    virtual void onTerminate() {}
    virtual ~ISSubscriber() = default;
    virtual bool onDataReceived(void *data) = 0;
    virtual void setPublisher(ISPublisher* publisher){
        listener_publisher = publisher;
    }
};

/**
 * Base class for Bridges. All implementation must inherit from it.
 */
class ISBridge
{
protected:
    ISPublisher *mp_publisher;
    ISSubscriber *ms_subscriber;
    ISPublisher *rtps_publisher;
    ISSubscriber *rtps_subscriber;
    //userf_t *transformation;
public:
    /**
     * This method will be called by ISManager when terminating the execution of the bridge.
     * Any handle, subscription, and resources that the bridge needed to work must be closed.
     */
    virtual void onTerminate()
    {
        if (mp_publisher)
        {
            mp_publisher->onTerminate();
        }

        if (ms_subscriber)
        {
            ms_subscriber->onTerminate();
        }

        if (rtps_publisher)
        {
            rtps_publisher->onTerminate();
        }

        if (rtps_subscriber)
        {
            rtps_subscriber->onTerminate();
        }
    }
    virtual ~ISBridge() = default;
    virtual ISPublisher* getOtherPublisher() { return mp_publisher; }
    virtual ISSubscriber* getOtherSubscriber() { return ms_subscriber; }
    virtual ISPublisher* getRTPSPublisher() { return rtps_publisher; }
    virtual ISSubscriber* getRTPSSubscriber() { return rtps_subscriber; }
    virtual void setOtherPublisher(ISPublisher *publisher) { mp_publisher = publisher; }
    virtual void setOtherSubscriber(ISSubscriber *subscriber) { ms_subscriber = subscriber; }
    virtual void setRTPSPublisher(ISPublisher *publisher) { rtps_publisher = publisher; }
    virtual void setRTPSSubscriber(ISSubscriber *subscriber) { rtps_subscriber = subscriber; }
    //virtual void setTransformation(userf_t *function) { transformation = function; }
};

#endif // _Header__SUBSCRIBER_H_
