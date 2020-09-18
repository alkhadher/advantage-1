/* ====================================================
 *
 *    (c) 2014 SAP AG or an SAP affiliate company. All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 *
 * ====================================================
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#if defined( _WIN32 )
   #include <windows.h>
#ifdef x64
   #define DEFAULT_LIBRARY_NAME "ace64.dll"
#else
   #define DEFAULT_LIBRARY_NAME "ace32.dll"
#endif
#else
   #include <dlfcn.h>
   #define DEFAULT_LIBRARY_NAME "libace.so"
#endif
#include "ace.h"
#include "adscapidll.h"

static
void * loadLibrary( const char * name )
/*************************************/
{
   void * handle;
#if defined( _WIN32 )
   handle = LoadLibrary( name );
#else
   handle = dlopen( name, RTLD_LAZY );
#endif
   return handle;
}

static
void unloadLibrary( void * handle )
/**********************************/
{
#if defined( _WIN32 )
   FreeLibrary( handle );
#else
   dlclose( handle );
#endif
}

static
void * findSymbol( void * dll_handle, const char * name )
/**************************************************/
{
#if defined( _WIN32 )
   return GetProcAddress( dll_handle, name );
#else
   return dlsym( dll_handle, name );
#endif
}

#define LookupSymbol( api, sym )             \
   api->sym = (sym ## _func)findSymbol( api->dll_handle, #sym );

#define LookupSymbolAndCheck( api, sym )           \
   api->sym = (sym ## _func)findSymbol( api->dll_handle, #sym );  \
   if( api->sym == NULL ) {               \
       unloadLibrary( api->dll_handle );           \
       return 0;                    \
   }

int ads_initialize_interface( AdvantageInterface * api, const char * path )
/*******************************************************************************/
{
   char * env;
   memset( api, 0, sizeof(*api));

   if ( path != NULL )
      {
      api->dll_handle = loadLibrary( path );
      if ( api->dll_handle != NULL )
         {
         goto loaded;
         }
      }
   env = getenv( "ADS_API_DLL" );
   if ( env != NULL )
      {
      api->dll_handle = loadLibrary( env );
      if ( api->dll_handle != NULL )
         {
         goto loaded;
         }
      }
   api->dll_handle = loadLibrary( DEFAULT_LIBRARY_NAME );
   if ( api->dll_handle != NULL )
      {
      goto loaded;
      }
   return 0;

   loaded:
   LookupSymbolAndCheck( api, ads_init );
   LookupSymbolAndCheck( api, ads_fini );
   LookupSymbolAndCheck( api, ads_new_connection );
   LookupSymbolAndCheck( api, ads_free_connection );
   LookupSymbolAndCheck( api, ads_make_connection );
   LookupSymbolAndCheck( api, ads_connect );
   LookupSymbolAndCheck( api, ads_disconnect );
   LookupSymbolAndCheck( api, ads_execute_immediate );
   LookupSymbolAndCheck( api, ads_prepare );
   LookupSymbolAndCheck( api, ads_free_stmt );
   LookupSymbolAndCheck( api, ads_num_params );
   LookupSymbolAndCheck( api, ads_describe_bind_param );
   LookupSymbolAndCheck( api, ads_bind_param );
   LookupSymbolAndCheck( api, ads_send_param_data );
   LookupSymbolAndCheck( api, ads_reset );
   LookupSymbolAndCheck( api, ads_get_bind_param_info );
   LookupSymbolAndCheck( api, ads_execute );
   LookupSymbolAndCheck( api, ads_execute_direct );
   LookupSymbolAndCheck( api, ads_fetch_absolute );
   LookupSymbolAndCheck( api, ads_fetch_next );
   LookupSymbolAndCheck( api, ads_get_next_result );
   LookupSymbolAndCheck( api, ads_affected_rows );
   LookupSymbolAndCheck( api, ads_num_cols );
   LookupSymbolAndCheck( api, ads_num_rows );
   LookupSymbolAndCheck( api, ads_get_column );
   LookupSymbolAndCheck( api, ads_get_data );
   LookupSymbolAndCheck( api, ads_get_data_info );
   LookupSymbolAndCheck( api, ads_get_column_info );
   LookupSymbolAndCheck( api, ads_commit );
   LookupSymbolAndCheck( api, ads_rollback );
   LookupSymbolAndCheck( api, ads_client_version );
   LookupSymbolAndCheck( api, ads_error );
   LookupSymbolAndCheck( api, ads_sqlstate );
   LookupSymbolAndCheck( api, ads_clear_error );
   LookupSymbolAndCheck( api, ads_cancel );
   LookupSymbolAndCheck( api, AdsBeginTransaction );

   api->initialized = 1;
   return 1;
}
#undef LookupSymbolAndCheck

void ads_finalize_interface( AdvantageInterface * api )
/***********************************************************/
{
   if ( !api->initialized )
      {
      return;
      }
   unloadLibrary( api->dll_handle );
   memset( api, 0, sizeof(*api));
}
