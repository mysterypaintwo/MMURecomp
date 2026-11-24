// FUN_005afed0 @ 0x5afed0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_005afed0(int param_1)

{
  char cVar1;
  bool bVar2;
  undefined4 uVar3;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *pbVar4;
  float10 fVar5;
  char *pcVar6;
  int local_34;
  undefined1 local_24 [8];
  undefined1 local_1c [4];
  undefined1 local_18 [12];
  int local_c;
  char local_5;
  
  cVar1 = FUN_005a82b0();
  if (cVar1 == '\0') {
    if ((*(char *)(param_1 + 0x3dc) != '\0') && (cVar1 = FUN_00441d90(), cVar1 != '\0')) {
      uVar3 = SDL_GetTicks();
      *(undefined4 *)(param_1 + 0x16c) = uVar3;
    }
    if ((*(float *)(param_1 + 0x60) == (float)_DAT_00697ce0) ||
       ((*(uint *)(param_1 + 0x98) & 0x10) == 0)) {
      if ((*(uint *)(param_1 + 0x98) & 0x10) == 0) {
        *(undefined4 *)(param_1 + 0x60) = 0;
      }
    }
    else {
      FUN_0043bc10(*(undefined4 *)(param_1 + 0x60),0);
      uVar3 = FUN_005a7400(0,0,0);
      uVar3 = FUN_00401930(uVar3);
      FUN_005b5220(local_1c,param_1,param_1 + 0x58,local_24,uVar3);
      FUN_00401510(local_18);
      if (local_c == 0) {
        if (*(float *)(param_1 + 0x60) <= (float)_DAT_00697ce0) {
          local_34 = 1;
        }
        else {
          local_34 = -1;
        }
        fVar5 = (float10)FUN_0043e510();
        *(float *)(param_1 + 0x60) =
             (float)(fVar5 * (float10)_DAT_00698340 * (float10)local_34 +
                    (float10)*(float *)(param_1 + 0x60));
      }
      else {
        *(undefined4 *)(param_1 + 0x60) = 0;
      }
    }
    local_5 = FUN_00441db0();
    if ((local_5 != '\0') || (cVar1 = FUN_00441db0(), cVar1 != '\0')) {
      if ((*(uint *)(param_1 + 0x98) & 8) == 0) {
        uVar3 = 0;
        cVar1 = FUN_00441db0(0);
        FUN_0043bc20((-(uint)(cVar1 != '\0') & 0xfffffffe) + 1,uVar3);
        if (*(char *)(param_1 + 0x19d) == '\0') {
          FUN_004405c0(&DAT_0067da08,param_1 + 4);
        }
        else {
          FUN_004405c0(&DAT_0067da10,param_1 + 4);
        }
      }
      else {
        *(char *)(param_1 + 0x68) = '\x01' - (local_5 != '\0');
      }
    }
    cVar1 = FUN_005aa080();
    if (cVar1 == '\0') {
      FUN_005ad160();
      pcVar6 = "ZeroShoryuken";
      pbVar4 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_0043d890();
      bVar2 = std::operator==<char,std::char_traits<char>,std::allocator<char>_>(pbVar4,pcVar6);
      if ((((!bVar2) && (cVar1 = FUN_005a9c40(1), cVar1 == '\0')) &&
          (cVar1 = FUN_005a9de0(), cVar1 == '\0')) && (cVar1 = FUN_005a97d0(1), cVar1 == '\0')) {
        FUN_005a87c0();
        FUN_005a8730();
      }
    }
  }
  return;
}

