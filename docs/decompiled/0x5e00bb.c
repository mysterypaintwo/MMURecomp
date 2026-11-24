// FUN_005df410 @ 0x5e00bb

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_005df410(void)

{
  byte bVar1;
  char cVar2;
  byte bVar3;
  HANDLE hProcess;
  uint _Seed;
  char *pcVar4;
  int extraout_EAX;
  int iVar5;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_8a4 [8];
  undefined4 uStack_89c;
  DWORD dwPriorityClass;
  undefined4 local_86c;
  int local_85c;
  CDHtmlElementEventSink *local_858;
  undefined4 *local_854;
  undefined4 local_850;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_830 [28];
  CDHtmlElementEventSink *local_814;
  CDHtmlElementEventSink *local_810;
  undefined1 *local_80c;
  void *local_808;
  undefined4 local_804;
  undefined4 *local_800;
  undefined4 *local_7fc;
  undefined1 *local_7f8;
  undefined1 *local_7f4;
  undefined1 *local_7f0;
  undefined1 *local_7ec;
  undefined4 local_7e8;
  undefined1 *local_7e4;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_7e0 [28];
  undefined1 *local_7c4;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_7c0 [28];
  undefined1 *local_7a4;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_7a0 [28];
  undefined1 *local_784;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_780 [28];
  undefined1 *local_764;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_760 [28];
  undefined1 *local_744;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_740 [28];
  undefined1 *local_724;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_720 [28];
  undefined1 *local_704;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_700 [28];
  undefined1 *local_6e4;
  undefined1 *local_6e0;
  undefined1 *local_6dc;
  undefined1 *local_6d8;
  undefined1 *local_6d4;
  undefined1 *local_6d0;
  undefined1 *local_6cc;
  undefined1 *local_6c8;
  undefined1 *local_6c4;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_6c0 [28];
  void *local_6a4;
  int local_6a0;
  void *local_69c;
  CDHtmlElementEventSink *local_698;
  void *local_694;
  undefined4 *local_690;
  undefined4 local_68c;
  undefined1 *local_688;
  void *local_684;
  undefined4 local_680;
  int local_668;
  basic_fstream<char,std::char_traits<char>_> local_664 [144];
  basic_fstream<char,std::char_traits<char>_> local_5d4 [144];
  basic_fstream<char,std::char_traits<char>_> local_544 [151];
  undefined1 local_4ad;
  basic_fstream<char,std::char_traits<char>_> local_4ac [151];
  undefined1 local_415;
  undefined1 local_414 [39];
  byte local_3ed;
  undefined1 local_3ec [112];
  undefined1 local_37c [60];
  undefined1 local_340 [48];
  int local_310;
  undefined1 local_30c [36];
  undefined1 local_2e8 [4];
  undefined1 local_2e4 [32];
  undefined1 local_2c4 [372];
  int local_150;
  basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> local_14c [28];
  undefined1 local_130 [32];
  undefined1 local_110 [44];
  undefined1 local_e4 [4];
  HMODULE local_e0;
  HMODULE local_dc;
  undefined1 local_d8 [32];
  CCmdUI local_b8 [40];
  HMODULE local_90;
  undefined1 local_8c [96];
  uint local_2c;
  undefined1 local_28 [36];
  
  local_2c = DAT_006c1e94 ^ (uint)(local_28 + 0x24);
  DAT_006c5758 = 0;
  DAT_006c55dc = 0;
  FUN_004405f0();
  DAT_006c55fc = local_8c;
  FUN_005dcaa0();
  FUN_00446c20();
  DAT_006c55cc = local_28;
  FUN_00447100();
  _DAT_006c55ec = local_d8;
  local_684 = operator_new(0x90);
  if (local_684 == (void *)0x0) {
    local_850 = 0;
  }
  else {
    local_850 = FUN_00405380();
  }
  local_680 = local_850;
  DAT_006c55e0 = local_850;
  cVar2 = FUN_00405360();
  if (cVar2 != '\0') {
    local_688 = abStack_8a4;
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_8a4,"Assets\\misc.mmu");
    FUN_0053f7c0();
  }
  PTR_DAT_006c1fd0 = &DAT_00682e44;
  CCmdUI::CCmdUI(local_b8);
  DAT_006c55d8 = local_b8;
  FUN_004f1030();
  cVar2 = FUN_005dc230();
  if (cVar2 == '\0') {
    local_68c = 1;
    FUN_004f0bd0();
    FUN_00447110();
    FUN_0043c1b0();
    FUN_00440670();
  }
  else {
    FUN_005dc9d0();
    FUN_00405500();
    FUN_005dc1e0();
    FUN_005dc5e0();
    local_90 = LoadLibraryA("ogg.dll");
    local_e0 = LoadLibraryA("vorbis.dll");
    local_dc = LoadLibraryA("vorbisfile.dll");
    dwPriorityClass = 0x8000;
    hProcess = GetCurrentProcess();
    SetPriorityClass(hProcess,dwPriorityClass);
    _Seed = FUN_005dc210();
    srand(_Seed);
    FUN_004472f0();
    _DAT_006c55b0 = local_37c;
    local_694 = operator_new(0x90);
    if (local_694 == (void *)0x0) {
      local_854 = (undefined4 *)0x0;
    }
    else {
      local_854 = (undefined4 *)FUN_00405380();
    }
    local_690 = local_854;
    DAT_006c55b4 = local_854;
    FUN_004e3c10();
    DAT_006c55c0 = local_130;
    local_69c = operator_new(0x20);
    if (local_69c == (void *)0x0) {
      local_858 = (CDHtmlElementEventSink *)0x0;
    }
    else {
      local_858 = (CDHtmlElementEventSink *)FUN_004e66d0();
    }
    local_698 = local_858;
    DAT_006c55a8 = local_858;
    FUN_0043b350();
    FUN_00425990();
    FUN_004431e0();
    DAT_006c55e8 = local_2e8;
    local_6a4 = operator_new(0x1e1c90);
    if (local_6a4 == (void *)0x0) {
      local_85c = 0;
    }
    else {
      local_85c = FUN_00445760();
    }
    local_6a0 = local_85c;
    DAT_006c55c4 = local_85c;
    FUN_00443d20();
    DAT_006c55b8 = local_414;
    FUN_005dd1c0();
    FUN_0061f950();
    DAT_006c55c8 = local_2e4;
    FUN_00562580();
    DAT_006c55bc = local_110;
    FUN_004462b0();
    DAT_006c55d4 = local_30c;
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_6c0,"savedgame.mmu");
    FUN_0061f6f0();
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_6c0);
    DAT_006c55a4 = local_2c4;
    local_6c4 = abStack_8a4;
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_8a4,"Camera");
    DAT_006c55f8 = (undefined4 *)FUN_00443e30();
    cVar2 = FUN_00405360();
    if (cVar2 == '\0') {
      local_6c8 = abStack_8a4;
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_8a4,"SoundGuy");
      DAT_006c559c = (undefined4 *)FUN_00443e30();
    }
    FUN_00405480();
    DAT_006c55d0 = local_340;
    cVar2 = FUN_00405360();
    if (cVar2 != '\0') {
      local_6cc = abStack_8a4;
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      basic_string<char,std::char_traits<char>,std::allocator<char>_>
                (abStack_8a4,"Assets\\cutscenedefs.mmu");
      FUN_0053e0b0();
    }
    FUN_0050bbb0();
    DAT_006c55a0 = local_3ec;
    cVar2 = FUN_00461380();
    if (cVar2 == '\0') {
      cVar2 = FUN_00405360();
      if (cVar2 != '\0') {
        local_7ec = abStack_8a4;
        std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
        basic_string<char,std::char_traits<char>,std::allocator<char>_>
                  (abStack_8a4,"Assets\\gameobjects.mmu");
        FUN_005403a0();
        local_7f0 = abStack_8a4;
        std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
        basic_string<char,std::char_traits<char>,std::allocator<char>_>
                  (abStack_8a4,"Assets\\cutscenes.mmu");
        FUN_005403a0();
        local_7f4 = abStack_8a4;
        std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
        basic_string<char,std::char_traits<char>,std::allocator<char>_>
                  (abStack_8a4,"Assets\\levelart.mmu");
        FUN_005403a0();
        local_7f8 = abStack_8a4;
        std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
        basic_string<char,std::char_traits<char>,std::allocator<char>_>
                  (abStack_8a4,"Assets\\menus.mmu");
        FUN_005403a0();
        local_800 = DAT_006c55b4;
        local_7fc = DAT_006c55b4;
        if (DAT_006c55b4 != (undefined4 *)0x0) {
          (**(code **)*DAT_006c55b4)();
        }
        uStack_89c = 0x5dfe57;
        std::basic_fstream<char,std::char_traits<char>_>::
        basic_fstream<char,std::char_traits<char>_>(local_5d4,"Assets\\levels.mmu",0x21,0x40);
        FUN_0044c530();
        std::basic_fstream<char,std::char_traits<char>_>::close(local_5d4);
        std::basic_fstream<char,std::char_traits<char>_>::_vbase_destructor_(local_5d4);
        uStack_89c = 0x5dfe98;
        std::basic_fstream<char,std::char_traits<char>_>::
        basic_fstream<char,std::char_traits<char>_>(local_664,"Assets\\guns.mmu",0x21,0x40);
        FUN_0044c530();
        local_668 = 0;
        while (CDHtmlElementEventSink::~CDHtmlElementEventSink(DAT_006c55a8),
              local_668 < extraout_EAX) {
          local_808 = operator_new(0x20);
          if (local_808 == (void *)0x0) {
            local_86c = 0;
          }
          else {
            FUN_00433b80();
            local_86c = FUN_0053c990();
          }
          local_804 = local_86c;
          local_668 = local_668 + 1;
        }
        local_80c = abStack_8a4;
        std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
        basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_8a4,"SoundGuy");
        DAT_006c559c = (undefined4 *)FUN_00443e30();
        (**(code **)(*(int *)DAT_006c55a8 + 0xc))();
        local_814 = DAT_006c55a8;
        local_810 = DAT_006c55a8;
        if (DAT_006c55a8 != (CDHtmlElementEventSink *)0x0) {
          (*(code *)**(undefined4 **)DAT_006c55a8)();
        }
        std::basic_fstream<char,std::char_traits<char>_>::close(local_664);
        std::basic_fstream<char,std::char_traits<char>_>::_vbase_destructor_(local_664);
      }
      local_310 = SDL_JoystickOpen();
      pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                       basic_string<char,std::char_traits<char>,std::allocator<char>_>
                                 (local_830,PTR_DAT_006c1fd0);
      std::operator+<char,std::char_traits<char>,std::allocator<char>_>(local_14c,pcVar4);
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_830);
      FUN_00619f10();
      DAT_006c55f4 = local_e4;
      local_150 = FUN_004bd9f0();
      for (local_3ed = 0; local_3ed == 0; local_3ed = bVar1 | bVar3 | local_3ed) {
        FUN_005dc5e0();
        FUN_0043e4b0();
        iVar5 = FUN_004bd9f0();
        if (local_150 != iVar5) {
          local_150 = FUN_004bd9f0();
          FUN_005dc230();
        }
        iVar5 = FUN_004613a0();
        if ((iVar5 != 0xc) && (iVar5 = FUN_004613a0(), iVar5 != 0xd)) {
          FUN_0043e510();
          FUN_00404b60();
        }
        FUN_00441620();
        FUN_00444d80();
        FUN_004464a0();
        FUN_00511350();
        FUN_005625d0();
        FUN_0043e750();
        FUN_00445770();
        FUN_0043e5a0();
        FUN_0043ea40();
        FUN_0043ebd0();
        FUN_00511bf0();
        FUN_004457d0();
        SDL_JoystickUpdate();
        while ((iVar5 = SDL_PollEvent(), bVar1 = local_3ed, iVar5 != 0 && (local_3ed == 0))) {
          cVar2 = FUN_005dc8b0();
          local_3ed = 1 - (cVar2 != '\0');
        }
        bVar3 = FUN_00441d90();
        local_3ed = bVar1 | bVar3;
        local_3ed = FUN_00404840();
      }
      if (DAT_006c55f8 != (undefined4 *)0x0) {
        (**(code **)*DAT_006c55f8)();
      }
      DAT_006c55f8 = (undefined4 *)0x0;
      if (DAT_006c559c != (undefined4 *)0x0) {
        (**(code **)*DAT_006c559c)();
      }
      DAT_006c559c = (undefined4 *)0x0;
      if (DAT_006c55c4 != 0) {
        FUN_004055e0();
      }
      DAT_006c55c4 = 0;
      if (local_310 != 0) {
        SDL_JoystickClose();
      }
      SDL_FreeSurface();
      SDL_Quit();
      FUN_00619ee0();
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      ~basic_string<char,std::char_traits<char>,std::allocator<char>_>
                ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)local_14c);
      FUN_00508140();
      FUN_0053e230();
      FUN_0061e890();
      FUN_004462d0();
      FUN_005627a0();
      FID_conflict__CAtlWinModule(local_2e4);
      FUN_00443d30();
      Mix_CloseAudio();
      FUN_004e37b0();
      ~_CancellationTokenCallback<>();
      FreeLibrary(local_dc);
      FreeLibrary(local_e0);
      FreeLibrary(local_90);
      ShowCursor(1);
      FUN_004f0bd0();
      FUN_00447110();
      FUN_0043c1b0();
      FUN_00440670();
    }
    else {
      FUN_00443f50();
      FUN_00620f80();
      local_6d0 = abStack_8a4;
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      basic_string<char,std::char_traits<char>,std::allocator<char>_>
                (abStack_8a4,"Assets\\gameobjects.mmu");
      FUN_0053f280();
      FUN_0050ca10();
      local_6d4 = abStack_8a4;
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      basic_string<char,std::char_traits<char>,std::allocator<char>_>
                (abStack_8a4,"Assets\\cutscenes.mmu");
      FUN_0053f280();
      FUN_00509dd0();
      local_6d8 = abStack_8a4;
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      basic_string<char,std::char_traits<char>,std::allocator<char>_>
                (abStack_8a4,"Assets\\levelart.mmu");
      FUN_0053f280();
      FUN_005090e0();
      local_6dc = abStack_8a4;
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      basic_string<char,std::char_traits<char>,std::allocator<char>_>
                (abStack_8a4,"Assets\\menus.mmu");
      FUN_0053f280();
      FUN_0053e640();
      local_6e0 = abStack_8a4;
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      basic_string<char,std::char_traits<char>,std::allocator<char>_>
                (abStack_8a4,"Assets\\cutscenedefs.mmu");
      FUN_0053e160();
      FUN_0050cfe0();
      uStack_89c = 0x5df9af;
      std::basic_fstream<char,std::char_traits<char>_>::basic_fstream<char,std::char_traits<char>_>
                (local_4ac,"Assets\\levels.mmu",0x32,0x40);
      local_415 = FUN_0044c8b0();
      std::basic_fstream<char,std::char_traits<char>_>::close(local_4ac);
      std::basic_fstream<char,std::char_traits<char>_>::_vbase_destructor_(local_4ac);
      uStack_89c = 0x5dfa1b;
      std::basic_fstream<char,std::char_traits<char>_>::basic_fstream<char,std::char_traits<char>_>
                (local_544,"Assets\\guns.mmu",0x32,0x40);
      local_4ad = FUN_0044c8b0();
      std::basic_fstream<char,std::char_traits<char>_>::close(local_544);
      std::basic_fstream<char,std::char_traits<char>_>::_vbase_destructor_(local_544);
      local_6e4 = abStack_8a4;
      pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                       basic_string<char,std::char_traits<char>,std::allocator<char>_>
                                 (local_700,PTR_DAT_006c1fd0);
      std::operator+<char,std::char_traits<char>,std::allocator<char>_>
                ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
                 abStack_8a4,pcVar4);
      FUN_00540270();
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_700);
      local_704 = abStack_8a4;
      pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                       basic_string<char,std::char_traits<char>,std::allocator<char>_>
                                 (local_720,PTR_DAT_006c1fd0);
      std::operator+<char,std::char_traits<char>,std::allocator<char>_>
                ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
                 abStack_8a4,pcVar4);
      FUN_00540270();
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_720);
      local_724 = abStack_8a4;
      pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                       basic_string<char,std::char_traits<char>,std::allocator<char>_>
                                 (local_740,PTR_DAT_006c1fd0);
      std::operator+<char,std::char_traits<char>,std::allocator<char>_>
                ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
                 abStack_8a4,pcVar4);
      FUN_00540270();
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_740);
      local_744 = abStack_8a4;
      pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                       basic_string<char,std::char_traits<char>,std::allocator<char>_>
                                 (local_760,PTR_DAT_006c1fd0);
      std::operator+<char,std::char_traits<char>,std::allocator<char>_>
                ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
                 abStack_8a4,pcVar4);
      FUN_00540270();
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_760);
      local_764 = abStack_8a4;
      pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                       basic_string<char,std::char_traits<char>,std::allocator<char>_>
                                 (local_780,PTR_DAT_006c1fd0);
      std::operator+<char,std::char_traits<char>,std::allocator<char>_>
                ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
                 abStack_8a4,pcVar4);
      FUN_00540270();
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_780);
      local_784 = abStack_8a4;
      pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                       basic_string<char,std::char_traits<char>,std::allocator<char>_>
                                 (local_7a0,PTR_DAT_006c1fd0);
      std::operator+<char,std::char_traits<char>,std::allocator<char>_>
                ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
                 abStack_8a4,pcVar4);
      FUN_00540270();
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_7a0);
      local_7a4 = abStack_8a4;
      pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                       basic_string<char,std::char_traits<char>,std::allocator<char>_>
                                 (local_7c0,PTR_DAT_006c1fd0);
      std::operator+<char,std::char_traits<char>,std::allocator<char>_>
                ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
                 abStack_8a4,pcVar4);
      FUN_00540270();
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_7c0);
      local_7c4 = abStack_8a4;
      pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                       basic_string<char,std::char_traits<char>,std::allocator<char>_>
                                 (local_7e0,PTR_DAT_006c1fd0);
      std::operator+<char,std::char_traits<char>,std::allocator<char>_>
                ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
                 abStack_8a4,pcVar4);
      FUN_00540270();
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_7e0);
      local_7e4 = abStack_8a4;
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      basic_string<char,std::char_traits<char>,std::allocator<char>_>
                (abStack_8a4,"Assets\\misc.mmu");
      FUN_0053f280();
      local_7e8 = 0;
      FUN_00508140();
      FUN_0053e230();
      FUN_0061e890();
      FUN_004462d0();
      FUN_005627a0();
      FID_conflict__CAtlWinModule(local_2e4);
      FUN_00443d30();
      Mix_CloseAudio();
      FUN_004e37b0();
      ~_CancellationTokenCallback<>();
      FUN_004f0bd0();
      FUN_00447110();
      FUN_0043c1b0();
      FUN_00440670();
    }
  }
  __security_check_cookie(local_2c ^ (uint)(local_28 + 0x24));
  return;
}

