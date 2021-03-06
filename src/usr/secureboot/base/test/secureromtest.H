/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/secureboot/base/test/secureromtest.H $                */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* COPYRIGHT International Business Machines Corp. 2013,2014              */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
#ifndef __SECUREROMTEST_H
#define __SECURETOMTEST_H


#include <sys/mm.h>
#include <sys/mmio.h>
#include <vfs/vfs.H>
#include <kernel/pagemgr.H>
#include <errl/errlentry.H>
#include <errl/errlmanager.H>
#include <secureboot/service.H>
#include <secureboot/secure_reasoncodes.H>

#include "../securerom.H"

extern trace_desc_t* g_trac_secure;

// Quick change for unit testing
//#define TRACUCOMP(args...)  TRACFCOMP(args)
#define TRACUCOMP(args...)


/**********************************************************************/
/*  UTILITY FUNCTIONS                                                 */
/*  -- note: these functions do not commit error logs                 */
/**********************************************************************/

// Moves signed files from PNOR to paged-in memory
errlHndl_t loadSignedFile( const char * i_signedFile_name,
                           void * & o_signedFile_pageAddr,
                           size_t & o_signedFile_size     );

// Safely removes signed files from memory
void unloadSignedFile( void * & io_signedFile_pageAddr,
                       size_t & io_signedFile_size     );


// @todo RTC:34080 - In future key hash will come from HW, but
// Get the software keys that match the signed container
void useSwKeyHash(sha2_hash_t * o_sw_key_hash);


/**********************************************************************/
/*  End of UTILITY FUNCTIONS                                          */
/**********************************************************************/



class SecureROMTest : public CxxTest::TestSuite
{
  public:

    /**
     * @brief Secure ROM Test - Verify a Signed Container
     */
    void test_verify(void)
    {
        TRACFCOMP(g_trac_secure,ENTER_MRK"SecureROMTest::test_verify>");

        errlHndl_t  l_errl  =   NULL;

        /*******************************************************************/
        /* Load "test_signed_container" from PNOR to use for verification  */
        /*******************************************************************/

        // Signed file variables
        const char * signedFile_name =   "test_signed_container";
        void *  signedFile_pageAddr  =   NULL;
        size_t  signedFile_size      =   0;

        // Call utility function
        l_errl = loadSignedFile( signedFile_name,
                                 signedFile_pageAddr,
                                 signedFile_size);

        if (l_errl)
        {
            TS_FAIL("SecureROMTest::test_verify: loadSignedFile() Failed");
            errlCommit(l_errl, SECURE_COMP_ID);
            return;
        }

        TRACUCOMP(g_trac_secure, "SecureROMTest::test_verify: "
                  "signedFile info: addr = %p, size=0x%x",
                  signedFile_pageAddr, signedFile_size);



        /*******************************************************************/
        /* @todo RTC:34080 - In future key hash will come from HW, but     */
        /*  for now, create and initialize a local SecureROM class and     */
        /*  and then override the hash key variable to use the SW Key Hash */
        /*  for the test_signed_container                                  */
        /*******************************************************************/
        SecureROM l_sRom;

        // Call initializeSecureROM()
        l_errl = l_sRom.initialize();

        if (l_errl)
        {
            TS_FAIL("SecureROMTest::test_verify: initializeSecureROM() Failed");
            errlCommit(l_errl, SECURE_COMP_ID);
            return;
        }

        // Use Utility Function to load software keys into iv_hash_key
        useSwKeyHash( & l_sRom.iv_hash_key);


        /*******************************************************************/
        /* Call verify function                                            */
        /*******************************************************************/

        // Warn about the exception being handled during verification
        printkd("test_verify(): expect to see 'mfsr r2 to CFAR handled': ");

        l_errl = l_sRom.verifyContainer( signedFile_pageAddr,
                                         signedFile_size     );

        if (l_errl)
        {
            TS_FAIL("SecureROMTest::test_verify: verifyContainer() Failed");
            errlCommit(l_errl, SECURE_COMP_ID);
            return;
        }


        /*******************************************************************/
        /* Unload "test_signed_container" from memory                      */
        /*******************************************************************/
        if ( signedFile_pageAddr != NULL   )
        {
            unloadSignedFile( signedFile_pageAddr, signedFile_size);
        }

        TRACFCOMP(g_trac_secure,EXIT_MRK"SecureROMTest::test_verify");

    };

};


/**********************************************************************/
/*  UTILITY FUNCTIONS                                                 */
/**********************************************************************/

// Moved test_signed_container from PNOR to paged-in memory
errlHndl_t loadSignedFile( const char * i_signedFile_name,
                                void * & o_signedFile_pageAddr,
                                size_t & o_signedFile_size     )
{

        errlHndl_t  l_errl  =   NULL;
        const char * l_signedFile_virtAddr  =   NULL;


        /*******************************************************************/
        /* Load file from PNOR to use for verification                     */
        /*******************************************************************/

        // Load file into virtual memory
        l_errl = VFS::module_load( i_signedFile_name );
        if (l_errl)
        {
            TRACFCOMP(g_trac_secure, "loadSignedFile(): Module "
                                     "Load FAILED: %s", i_signedFile_name);

            return l_errl;
        }

        // Get memory address of file
        l_errl = VFS::module_address ( i_signedFile_name,
                                       l_signedFile_virtAddr,
                                       o_signedFile_size);
        if (l_errl)
        {
            TRACFCOMP(g_trac_secure, "loadSignedFile()> Module "
                                     "Address FAILED: %s", i_signedFile_name);

            return l_errl;
        }


        // Request contiguous memory block to copy in file
        size_t l_num_pages = ALIGN_PAGE(o_signedFile_size)/PAGESIZE;
        bool l_isUserspace = true;
        o_signedFile_pageAddr = PageManager::allocatePage(l_num_pages,
                                                          l_isUserspace);

        //  memcpy the file to allocated pages
        memcpy( o_signedFile_pageAddr, l_signedFile_virtAddr,
                o_signedFile_size );

        TRACUCOMP(g_trac_secure, "loadSignedFile()> signedFile '%s' "
                  "Info: sF_pA=%p, sF_vA=%p, size=0x%x (pages=%d)",
                  i_signedFile_name, o_signedFile_pageAddr,
                  l_signedFile_virtAddr, o_signedFile_size, l_num_pages);

    return l_errl;
}

// Safely removes signed files from memory
void unloadSignedFile( void * & io_signedFile_pageAddr,
                       size_t & io_signedFile_size     )
{

        // Determine number of pages to be freed
        size_t l_num_pages = ALIGN_PAGE(io_signedFile_size)/PAGESIZE;

        // Free page(s)
        PageManager::freePage(io_signedFile_pageAddr, l_num_pages);

        // Reset pageAddr pointer
        io_signedFile_pageAddr = NULL;

        TRACUCOMP(g_trac_secure, "unloadSignedFile()> "
                  "Info: sF_pA=%p, size=0x%x (pages=%d)",
                  io_signedFile_pageAddr, io_signedFile_size, l_num_pages);

}


// @todo RTC:34080 - In future key hash will come from HW, but
// Get the software keys that match the signed container
void useSwKeyHash(sha2_hash_t * o_sw_key_hash)
{


    /* On HW hw_key_hash will be pulled from SBE's SEEPROM or PIB Memory */

    /* For simics/testing, we're hardcoding the value from Dale        */
    /*  Peterson here, which I've stored here:                         */
    /* /gsa/rchgsa-h1/00/baiocchi/HB/Story_Notes/Story_64764/          */
    /*     Secure_ROM_Files/HB_SW_Files/hdwkeyHash                     */

    /* Dale's files' hw-hash-key */
    uint64_t l_hdwkeyHash[] = { 0x6c8206fa4f551088, 0xc8c23af693ed242d,
                                0x96824dac71bccf07, 0x73abb342d4bb932f,
                                0x9a9a8d72fbf28202, 0xd89fa8fad5658959,
                                0xc89ccf0d0205b8ca, 0x2071ba27e18ae0f8
                              };

    // Copy l_hdwkeyHash into sha2_hash_t
    memcpy (o_sw_key_hash, &l_hdwkeyHash, sizeof(sha2_hash_t));

    return;
}


#endif
