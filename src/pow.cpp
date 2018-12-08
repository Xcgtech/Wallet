// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pow.h"

#include "arith_uint256.h"
#include "consensus/params.h"
#include "chain.h"
#include "chainparams.h"
#include "primitives/block.h"
#include "uint256.h"
#include "util.h"
#include "streams.h"

#include <cmath>
#include <math.h>
#include <algorithm>
#include <iostream>


unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{

  assert(pindexLast != nullptr);
  int nHeight = pindexLast->nHeight + 1;
    // Most recent algo first

  //  if (pindexLast->nHeight + 1 >= params.nDigishieldAveragingWindow) {
  //    return DigishieldGetNextWorkRequired(pindexLast, pblock, params);
  //  }
     if (pindexLast->nHeight + 1 >= params.XCGZawyLWMAHeight) {
      return LwmaGetNextWorkRequired(pindexLast, pblock, params);
    }
    else {
        return GetNextWorkRequiredBTC(pindexLast, pblock, params);
    }
}

unsigned int LwmaGetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
     if(params.XbufferTSA<pindexLast->nHeight)  {
      return XbufferTSA(pblock,params,pindexLast);}
     else{  
      return LwmaCalculateNextWorkRequired(pindexLast,params);}
}

unsigned int XbufferTSA(const CBlockHeader* pblock, const Consensus::Params& consensusParams, const CBlockIndex* pindexPrev)
{ // Xbuffer Xchange (c) 2018 Kalcan
  // TSA Copyright (c) 2018 Zawy, Kalcan (XCG)
  // LWMA Copyright (c) 2017-2018 The Bitcoin Gold developers
  // Specific LWMA updated by iamstenman (MicroBitcoin)
  // MIT License
  // Recommend N=120.  Requires Future Time Limit to be set to 30 instead of 7200
  // See FTL instructions here:
  // https://github.com/zawy12/difficulty-algorithms/issues/3#issuecomment-442129791
  // Important: Pow.cpp is not the only area that needs new conditions or code for TSA.    
  // ONLY LWMA and TSA copy between the #######'s
  //################################################################################################################
  // Begin LWMA

      const int64_t T = consensusParams.nPowTargetSpacing;
      const int64_t N = consensusParams.nZawyLwmaAveragingWindow;
      const int64_t k = N * (N + 1) * T / 2;
      const int64_t height = pindexPrev->nHeight;
      const arith_uint256 powLimit = UintToArith256(consensusParams.powLimit);

      // For startup:
      int64_t hashrateGuess = 1000; // hashes per second expected at startup
      arith_uint256 targetGuess = 115E75/(hashrateGuess*T); // 115E75 =~ pow(2,256)
      if ( targetGuess > powLimit ) { targetGuess=powLimit; }
      if (height <= N+1) { return targetGuess.GetCompact(); }

      arith_uint256 sumTarget, previousTarget, nextTarget;
      int64_t thisTimestamp, previousTimestamp;
      int64_t t = 0, j = 0, SumWAvgtime=0, SS=0;   // Remove SS if following only LWMA and TSA

      const CBlockIndex* blockPreviousTimestamp = pindexPrev->GetAncestor(height - N);
      previousTimestamp = blockPreviousTimestamp->GetBlockTime();

      // Loop through N most recent blocks. // LWMA previous target-> bits and Timestamps are the focys
      for (int64_t i = height - N + 1; i <= height; i++) {
          const CBlockIndex* block = pindexPrev->GetAncestor(i);
          thisTimestamp = (block->GetBlockTime() > previousTimestamp) ? block->GetBlockTime() : previousTimestamp + 1;
          int64_t solvetime = std::min(6 * T, thisTimestamp - previousTimestamp);
          previousTimestamp = thisTimestamp;
          SS+=solvetime;
          j++;
          t += solvetime * j; // Weighted solvetime sum.
          arith_uint256 target;
          target.SetCompact(block->nBits);
          sumTarget += target / (k * N);
      }
      nextTarget = t * sumTarget;

    // The above completes the BTG/MicroBitcoin/Zawy code except last 3 lines were removed.
    // Now begin TSA.

     // R is the "softness" of the per-block TSA adjustment to the DA. R<6 is aggressive.
     int64_t R = 2;  assert(R > 1);
     // "m" is a factor to help get e^x from integer math. 1E5 was not as precise
     int64_t m = 1E6;
     arith_uint256  TSATarget;
     int64_t exm = m;  // This will become m*e^x. Initial value is m*e^(mod(<1)) = m.
     int64_t SC, ST, i, f; //remove SC for TSA & LWMA
     int64_t templateTimestamp = pblock->nTime;
     previousTimestamp = pindexPrev->GetBlockTime();
     const CBlockIndex* badblockcheck = pindexPrev->GetAncestor(height-1); //Only for Xbuffer
  
     ST = std::min(templateTimestamp - previousTimestamp, 6*T);
  //################################################################################################################
  //-------------------------------------------------Xbuffer--------------------------------------------------------
       SC=ST;//real solvetime checkpoint
       if(SS/N+1<=T/R){SS=(SS/(N+1)/T)*SS;}//if your avg solvetime is abnormally low or a new coin, drop SS to increase Buffer ST
                                           // resultihng in a faster pull towards the real T
       ST = (ST*((SS/(N+1)*1000)/T))/1000;// find the ratio real ST:Sum ST && and use it on the real ST.
  
      if(SC>T/R&&ST==0){ST=SC;}// checkpoint for new coins and network issues - don't remove, keep for new coins.

      if((previousTimestamp-badblockcheck->GetBlockTime())<=(T/R)+R&&ST<T){ // Xbuffer - Mining equalizer. If ST < T/2+R then Next block is a free-fall block. 
         TSATarget=nextTarget*(1/T);}                                      // Old Miners benefit from new H/S increase
                                                                                                       
      else if(ST<T/R||ST>T){TSATarget=nextTarget*((ST/T)+(10/T));}  //The Value 10 is user defined, relates to  speed of diff change on the Xbuffer                               
                                                                   //sets min and max.
                                                                   //ST will not be constant unti SS/N+1 = T
                                                                  //End of solve time will be a free-fall block
     else{  
   //TSA in a pocket based on the length of T/R to allow fair mining based on raw H/S for Xbuffer.
      int64_t T2=T/R;
      ST = ST-((T/R)-1);
    //-------------------------------------------------Xbuffer--------------------------------------------------------
  //################################################################################################################
      // relpace all T2 with T if excluding Xbuffer 
     // It might be good to turn this e^x equation into a look-up table;
     for ( i = 1; i <= ST/T2/R ; i++ ) {
          exm = (exm*static_cast<int64_t>(2.71828*m))/m;
         }
     f = ST % (T2*R);
     exm = (exm*(m+(f*(m+(f*(m+(f*(m+(f*m)/(4*T2*R)))/(3*T2*R)))/(2*T2*R)))/(T2*R)))/m;

     // 1000 below is to prevent overflow on testnet
     TSATarget = (nextTarget*((1000*(m*T2+(ST-T2)*exm))/(m*ST)))/1000;
        } // No Xbuffer remove
     if (TSATarget > powLimit) { TSATarget = powLimit; }
      //LogPrintf("LWMA: %s\n  TSA: %s\n Diff: %s\n height: %s\n exm: %s\n f: %s\n, currentT:%s\n",nextTarget.GetHex(), TSATarget.GetHex(), GetDifficulty(pindexPrev), height, previousTimestamp, ST, templateTimestamp);
    return TSATarget.GetCompact();


}
unsigned int LwmaCalculateNextWorkRequired(const CBlockIndex* pindexLast, const Consensus::Params& params)
{


  if (params.fPowNoRetargeting) {  return pindexLast->nBits;   }
      const int64_t FTL = 450;
   const int64_t T = params.nPowTargetSpacing;
   const int64_t N = params.nZawyLwmaAveragingWindow;
   const int64_t k =  N*(N+1)*T/2; ;
   const int height = pindexLast->nHeight + 1;
   assert(height > N);

   arith_uint256 sum_target;
   int t = 0, j = 0, solvetime;
   // Loop through N most recent blocks.
   // [zawy edit: height - 1 is the most recently solved block
   for (int i = height - N+1; i < height; i++) {
       const CBlockIndex* block = pindexLast->GetAncestor(i);
       const CBlockIndex* block_Prev = block->GetAncestor(i - 1);
       int64_t solvetime = block->GetBlockTime() - block_Prev->GetBlockTime();
       solvetime = std::max(-FTL, std::min(solvetime, 6*T));


       j++;
       t += solvetime * j;  // Weighted solvetime sum.

    
       arith_uint256 target;
       target.SetCompact(block->nBits);
       sum_target += target / (k * N);

   }
   // Keep t reasonable in case strange solvetimes occurred.
   if (t < k/10) {   t = k/10;      }
//  const arith_uint256 pow_limit = UintToArith256(params.powLimit);
   arith_uint256 next_target = t * sum_target;
//   if (next_target > pow_limit) {   next_target = pow_limit;     }

   return next_target.GetCompact();
}

/*unsigned int DigishieldGetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock,
                                           const Consensus::Params& params)
{
    assert(pindexLast != nullptr);
    unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();  // Always postfork.

    const CBlockIndex* pindexFirst = pindexLast;
    arith_uint256 bnTot {0};
    for (int i = 0; pindexFirst && i < params.nDigishieldAveragingWindow; i++) {
        arith_uint256 bnTmp;
        bnTmp.SetCompact(pindexFirst->nBits);
        bnTot += bnTmp;
        pindexFirst = pindexFirst->pprev;
    }

    if (pindexFirst == NULL)
        return nProofOfWorkLimit;

    arith_uint256 bnAvg {bnTot / params.nDigishieldAveragingWindow};
    return DigishieldCalculateNextWorkRequired(bnAvg, pindexLast, pindexFirst, params);
}

unsigned int DigishieldCalculateNextWorkRequired(arith_uint256 bnAvg, const CBlockIndex* pindexLast, const CBlockIndex* pindexFirst, const Consensus::Params& params)
{
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    int64_t nLastBlockTime = pindexLast->GetMedianTimePast();
    int64_t nFirstBlockTime = pindexFirst->GetMedianTimePast();
    // Limit adjustment
    int64_t nActualTimespan = nLastBlockTime - nFirstBlockTime;

    if (nActualTimespan < params.DigishieldMinActualTimespan())
        nActualTimespan = params.DigishieldMinActualTimespan();
    if (nActualTimespan > params.DigishieldMaxActualTimespan())
        nActualTimespan = params.DigishieldMaxActualTimespan();

    // Retarget
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    arith_uint256 bnNew {bnAvg};
    bnNew /= params.DigishieldAveragingWindowTimespan();
    bnNew *= nActualTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
}
*/
unsigned int GetNextWorkRequiredBTC(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

    // Genesis block
    if (pindexLast == NULL)
        return nProofOfWorkLimit;

    // Only change once per interval
    if ((pindexLast->nHeight+1) % params.DifficultyAdjustmentInterval() != 0)
    {
        if (params.fPowAllowMinDifficultyBlocks)
        {
            // Special difficulty rule for testnet:
            // If the new block's timestamp is more than 2* 2.5 minutes
            // then allow mining of a min-difficulty block.
            if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing*2)
                return nProofOfWorkLimit;
            else
            {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex* pindex = pindexLast;
                while (pindex->pprev && pindex->nHeight % params.DifficultyAdjustmentInterval() != 0 && pindex->nBits == nProofOfWorkLimit)
                    pindex = pindex->pprev;
                return pindex->nBits;
            }
        }
        return pindexLast->nBits;
    }

    // Go back by what we want to be 1 day worth of blocks
    int nHeightFirst = pindexLast->nHeight - (params.DifficultyAdjustmentInterval()-1);
    assert(nHeightFirst >= 0);
    const CBlockIndex* pindexFirst = pindexLast->GetAncestor(nHeightFirst);
    assert(pindexFirst);

   return CalculateNextWorkRequired(pindexLast, pindexFirst->GetBlockTime(), params);
}



// for DIFF_BTC only!
unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    LogPrintf("  nActualTimespan = %d  before bounds\n", nActualTimespan);
    if (nActualTimespan < params.nPowTargetTimespan/4)
        nActualTimespan = params.nPowTargetTimespan/4;
    if (nActualTimespan > params.nPowTargetTimespan*4)
        nActualTimespan = params.nPowTargetTimespan*4;

    // Retarget
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    arith_uint256 bnNew;
    arith_uint256 bnOld;
    bnNew.SetCompact(pindexLast->nBits);
    bnOld = bnNew;
    bnNew *= nActualTimespan;
    bnNew /= params.nPowTargetTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    /// debug print
    LogPrintf("GetNextWorkRequired RETARGET\n");
    LogPrintf("params.nPowTargetTimespan = %d    nActualTimespan = %d\n", params.nPowTargetTimespan, nActualTimespan);
    LogPrintf("Before: %08x  %s\n", pindexLast->nBits, bnOld.ToString());
    LogPrintf("After:  %08x  %s\n", bnNew.GetCompact(), bnNew.ToString());

    return bnNew.GetCompact();
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
        return error("CheckProofOfWork(): nBits below minimum work");

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return error("CheckProofOfWork(): hash doesn't match nBits");

    return true;
}

arith_uint256 GetBlockProof(const CBlockIndex& block)
{
    arith_uint256 bnTarget;
    bool fNegative;
    bool fOverflow;
    bnTarget.SetCompact(block.nBits, &fNegative, &fOverflow);
    if (fNegative || fOverflow || bnTarget == 0)
        return 0;
    // We need to compute 2**256 / (bnTarget+1), but we can't represent 2**256
    // as it's too large for a arith_uint256. However, as 2**256 is at least as large
    // as bnTarget+1, it is equal to ((2**256 - bnTarget - 1) / (bnTarget+1)) + 1,
    // or ~bnTarget / (nTarget+1) + 1.
    return (~bnTarget / (bnTarget + 1)) + 1;
}

int64_t GetBlockProofEquivalentTime(const CBlockIndex& to, const CBlockIndex& from, const CBlockIndex& tip, const Consensus::Params& params)
{
    arith_uint256 r;
    int sign = 1;
    if (to.nChainWork > from.nChainWork) {
        r = to.nChainWork - from.nChainWork;
    } else {
        r = from.nChainWork - to.nChainWork;
        sign = -1;
    }
    r = r * arith_uint256(params.nPowTargetSpacing) / GetBlockProof(tip);
    if (r.bits() > 63) {
        return sign * std::numeric_limits<int64_t>::max();
    }
    return sign * r.GetLow64();
}
