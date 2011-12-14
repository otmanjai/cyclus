// NullMarket.cpp
// Implements the NullMarket class
#include <iostream>
#include "Logger.h"

#include "NullMarket.h"

#include "Logician.h"
#include "CycException.h"
#include "InputXML.h"

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  
void NullMarket::receiveMessage(Message *msg) {
  messages_.insert(msg);

  // offers and requests are ordered by amount
  if (msg->isOffer()){
    offers_.insert(indexedMsg(msg->getResource()->getQuantity(),msg));
  }
  else if (!msg->isOffer()){
    requests_.insert(indexedMsg(-msg->getResource()->getQuantity(),msg));
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  
void NullMarket::reject_request(sortedMsgList::iterator request)
{

  // delete the tentative orders
  while ( orders_.size() > firmOrders_)
  {
    delete orders_.back();
    orders_.pop_back();
  }

  // put all matched offers_ back in the sorted list
  while (matchedOffers_.size() > 0)
  {
    Message* msg = *(matchedOffers_.begin());
    offers_.insert(indexedMsg(msg->getResource()->getQuantity(),msg));
    matchedOffers_.erase(msg);
  }

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  
void NullMarket::process_request()
{
  // update pointer to firm orders
  firmOrders_ = orders_.size();

  while (matchedOffers_.size() > 0)
  {
    Message* msg = *(matchedOffers_.begin());
    messages_.erase(msg);
    matchedOffers_.erase(msg);
  }
}
 
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  
bool NullMarket::match_request(sortedMsgList::iterator request)
{
  sortedMsgList::iterator offer;
  double requestAmt,offerAmt, toRet;
  Message *offerMsg, *requestMsg;

  requestAmt = request->first;
  requestMsg = request->second;
  
  // if this request is not yet satisfied &&
  // there are more offers_ left
  while ( requestAmt > 0 && offers_.size() > 0)
  {
    // get a new offer
    offer = offers_.end();
    offer--;
    offerAmt = (*offer).first;
    offerMsg = (*offer).second;

    // pop off this offer
    offers_.erase(offer);
  
    if (requestAmt > offerAmt) { 
      // put a new message in the order stack
      // it goes down to supplier
      offerMsg->setRequester(requestMsg->getRequester());

      // Queue an order
      matchedOffers_.insert(offerMsg);
      // Zero out the boolean. 
      // At least some of the request will be met.
      toRet = 0;

      orders_.push_back(offerMsg);

      LOG(LEV_DEBUG2) << "NullMarket has resolved a match from "
          << offerMsg->getSupplier()->ID()
          << " to "
          << offerMsg->getRequester()->ID()
          << " for the amount:  " 
          << offerMsg->getResource()->getQuantity();

      requestAmt -= offerAmt;
    } 
    else {
      // split offer

      // queue a new order
      Message* maybe_offer = offerMsg->clone();
      maybe_offer->getResource()->setQuantity(requestAmt);
      maybe_offer->setRequester(requestMsg->getRequester());

      matchedOffers_.insert(offerMsg);

      orders_.push_back(maybe_offer);

      LOG(LEV_DEBUG2) << "NullMarket has resolved a match from "
          << maybe_offer->getSupplier()->ID()
          << " to "
          << maybe_offer->getRequester()->ID()
          << " for the amount:  " 
          << maybe_offer->getResource()->getQuantity();

      // reduce the offer amount
      offerAmt -= requestAmt;

      // if the residual is above threshold,
      // make a new offer with reduced amount

      if(offerAmt > EPS_KG){
        Message* new_offer = offerMsg->clone();
        new_offer->getResource()->setQuantity(offerAmt);
        // call this method for consistency
        receiveMessage(new_offer);
      }
      
      // zero out request
      requestAmt = 0;

      // Zero out the boolean. 
      // All of the request will be met.
      toRet = 0;
    }
  }

  // if any of the request has been met, return true.
  return (0 == toRet);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  
void NullMarket::resolve()
{
  sortedMsgList::iterator request;

  firmOrders_ = 0;

  /// while requests_ remain and there is at least one offer left
  while (requests_.size() > 0)
  {
    request = requests_.end();
    request--;
    
    if(match_request(request)) {
      process_request();
    } 
    else {
      LOG(LEV_DEBUG2) << "The request from Requester "<< (*request).second->getRequester()->ID()
          << " for the amount " << (*request).first 
          << " rejected. ";
      reject_request(request);
    }
    // remove this request
    messages_.erase((*request).second);
    requests_.erase(request);
  }

  for (int i = 0; i < orders_.size(); i++) {
    Message* msg = orders_.at(i);
    msg->setDir(DOWN_MSG);
    msg->sendOn();
  }
  orders_.clear();
}
