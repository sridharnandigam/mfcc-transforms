#include "FIRQ7.h"
#include <stdio.h>
#include "Error.h"

#define SNR_THRESHOLD 10

#define ABS_ERROR_Q7 ((q7_t)2)

#if defined(ARM_MATH_MVEI)
static __ALIGNED(8) q7_t coeffArray[32];
#endif 

void checkInnerTail(q7_t *b)
{
    ASSERT_TRUE(b[0] == 0);
    ASSERT_TRUE(b[1] == 0);
    ASSERT_TRUE(b[2] == 0);
    ASSERT_TRUE(b[3] == 0);
}

    void FIRQ7::test_fir_q7()
    {
        

        const int16_t *configp = configs.ptr();
        q7_t *statep = state.ptr();
        const q7_t *orgcoefsp = coefs.ptr();

        const q7_t *coefsp;
        const q7_t *inputp = inputs.ptr();
        q7_t *outp = output.ptr();

        int i;
#if defined(ARM_MATH_MVEI)
        int j;
#endif
        int blockSize;
        int numTaps;

        /*

        Python script is generating different tests with
        different blockSize and numTaps.

        We loop on those configs.

        */
        for(i=0; i < configs.nbSamples() >> 1; i++)
        {
           blockSize = configp[0];
           numTaps = configp[1];

#if defined(ARM_MATH_MVEI)
           /* Copy coefficients and pad to zero 
           */
           memset(coeffArray,0,32);
           for(j=0;j < numTaps; j++)
           {
              coeffArray[j] = orgcoefsp[j];
           }
   
           coefsp = coeffArray;
#else
           coefsp = orgcoefsp;
#endif

           /*

           The filter is initialized with the coefs, blockSize and numTaps.

           */
           arm_fir_init_q7(&this->S,numTaps,coefsp,statep,blockSize);

           /*

           Input pointer is reset since the same input pattern is used

           */
           inputp = inputs.ptr();

           /*
           
           Python script is filtering a 2*blockSize number of samples.
           We do the same filtering in two pass to check (indirectly that
           the state management of the fir is working.)

           */
           arm_fir_q7(&this->S,inputp,outp,blockSize);
           outp += blockSize;
           checkInnerTail(outp);

           inputp += blockSize;
           arm_fir_q7(&this->S,inputp,outp,blockSize);
           outp += blockSize;
           checkInnerTail(outp);

           configp += 2;
           orgcoefsp += numTaps;

        }

        ASSERT_EMPTY_TAIL(output);

        ASSERT_SNR(output,ref,(q7_t)SNR_THRESHOLD);

        ASSERT_NEAR_EQ(output,ref,ABS_ERROR_Q7);

    } 

 
    void FIRQ7::setUp(Testing::testID_t id,std::vector<Testing::param_t>& params,Client::PatternMgr *mgr)
    {
      
       
       switch(id)
       {
        case FIRQ7::TEST_FIR_Q7_1:
          
        break;

       }
      

       inputs.reload(FIRQ7::FIRINPUTS_Q7_ID,mgr);
       coefs.reload(FIRQ7::FIRCOEFS_Q7_ID,mgr);
       configs.reload(FIRQ7::FIRCONFIGS_S16_ID,mgr);
       ref.reload(FIRQ7::FIRREFS_Q7_ID,mgr);

       output.create(ref.nbSamples(),FIRQ7::OUT_Q7_ID,mgr);
       /* Max blockSize + numTaps - 1 as generated by Python script */
       state.create(47,FIRQ7::OUT_Q7_ID,mgr);
    }

    void FIRQ7::tearDown(Testing::testID_t id,Client::PatternMgr *mgr)
    {
        output.dump(mgr);
    }