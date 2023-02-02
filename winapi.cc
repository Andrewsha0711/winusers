#ifndef UNICODE
#define UNICODE
#endif

#include <napi.h>
#include <windows.h>
#include <stdio.h>
#include <lm.h>
#include <lmaccess.h>
#include <iostream>

#pragma comment(lib, "netapi32.lib")

Napi::String Check(const Napi::CallbackInfo &info)
{
   // because my system uses the russian locale
   setlocale(LC_ALL, "Russian");

   Napi::Env env = info.Env();
   // username from arg
   std::string s = info[0].As<Napi::String>();
   // user groups
   std::string groups = "";

   std::wstring stemp = std::wstring(s.begin(), s.end());
   LPCWSTR username = stemp.c_str();

   LPLOCALGROUP_USERS_INFO_0 pBuf = NULL;
   DWORD dwLevel = 0;
   DWORD dwFlags = LG_INCLUDE_INDIRECT;
   DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
   DWORD dwEntriesRead = 0;
   DWORD dwTotalEntries = 0;
   NET_API_STATUS nStatus;

   nStatus = NetUserGetLocalGroups(NULL, // localhost
                                   username,
                                   dwLevel,
                                   dwFlags,
                                   (LPBYTE *)&pBuf,
                                   dwPrefMaxLen,
                                   &dwEntriesRead,
                                   &dwTotalEntries);
   int status = 0;
   if (nStatus == NERR_Success)
   {
      LPLOCALGROUP_USERS_INFO_0 pTmpBuf;
      DWORD i;
      DWORD dwTotalCount = 0;

      if ((pTmpBuf = pBuf) != NULL)
      {
         for (i = 0; i < dwEntriesRead; i++)
         {
            if (pTmpBuf == NULL)
            {
               fprintf(stderr, "An access violation has occurred\n");
               break;
            }
            if (wcscmp(pTmpBuf->lgrui0_name, L"Администраторы") == 0)
               groups.append("admin ");
            if (wcscmp(pTmpBuf->lgrui0_name, L"Пользователи") == 0)
               groups.append("user ");
            pTmpBuf++;
            dwTotalCount++;
            status++;
         }
      }
   }
   else
      status = -1;
   if (pBuf != NULL)
      NetApiBufferFree(pBuf);
   if (status > 0)
      return Napi::String::New(env, groups);
   else
      return Napi::String::New(env, "not found");
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
   exports.Set(Napi::String::New(env, "check"),
               Napi::Function::New(env, Check));
   return exports;
}

NODE_API_MODULE(addon, Init)