// CreateObject @ 0x41dbb0

/* Library Function - Multiple Matches With Same Base Name
    public: static class CNoTrackObject * __cdecl CThreadLocal<class
   _AFX_THREAD_STATE>::CreateObject(void)
    public: static class CObject * __cdecl CFrameWnd::CreateObject(void)
   
   Libraries: Visual Studio 2005 Debug, Visual Studio 2008 Debug */

undefined4 CreateObject(void)

{
  uint uVar1;
  void *pvVar2;
  undefined4 local_1c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0064416b;
  local_10 = ExceptionList;
  uVar1 = DAT_006c1e94 ^ (int)register0x00000010 - 4U;
  ExceptionList = &local_10;
  pvVar2 = operator_new(0x1bc);
  local_8 = 0;
  if (pvVar2 == (void *)0x0) {
    local_1c = 0;
  }
  else {
    local_1c = FUN_00540f70(uVar1);
  }
  ExceptionList = local_10;
  return local_1c;
}

