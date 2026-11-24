// FUN_005262c0 @ 0x5262c0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_005262c0(int param_1)

{
  float fVar1;
  float fVar2;
  bool bVar3;
  char cVar4;
  int iVar5;
  int iVar6;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *pbVar7;
  uint uVar8;
  int *piVar9;
  float10 fVar10;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_9c [8];
  undefined4 uStack_94;
  undefined1 *puStack_90;
  char *pcStack_8c;
  char *pcVar11;
  undefined1 local_58 [16];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_48 [28];
  undefined1 *local_2c;
  int local_28;
  undefined1 local_24 [8];
  undefined4 local_1c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00642b8f;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0053b2f0();
  if ((((*(int *)(param_1 + 4) == 0xd) && (*(int *)(param_1 + 0x10) == 0)) &&
      (iVar5 = FUN_00403bf0(), iVar5 == 0)) && (*(char *)(param_1 + 0xa0) == '\0')) {
    FUN_0043bc00();
    local_2c = abStack_9c;
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_9c,"Zero");
    local_1c = FUN_00443e30();
    FUN_00430340();
    *(undefined4 *)(param_1 + 0x10) = local_1c;
    *(undefined1 *)(param_1 + 0xa0) = 1;
  }
  if (*(int *)(param_1 + 4) == 0x13) {
    FUN_00441b70();
    FUN_00462a90();
    iVar5 = FUN_00401930();
    iVar6 = FUN_00401930();
    if ((float)_DAT_00698f98 < *(float *)(iVar5 + 4) - *(float *)(iVar6 + 4)) {
      pcVar11 = "Stand";
      FUN_00462a90();
      pbVar7 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_0043d890();
      pcStack_8c = (char *)0x5263cb;
      bVar3 = std::operator==<char,std::char_traits<char>,std::allocator<char>_>(pbVar7,pcVar11);
      if (bVar3) {
        cVar4 = FUN_00461870();
        if (cVar4 == '\0') {
          std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
          basic_string<char,std::char_traits<char>,std::allocator<char>_>
                    (local_48,"..\\Cutscenes\\CSNZero\\csnWily5Outro.xcut");
          local_8 = 0;
          FUN_00621890();
          local_8 = 0xffffffff;
          std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
          ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_48);
        }
        else {
          cVar4 = FUN_004ad6a0();
          if (cVar4 == '\0') {
            FUN_006235b0();
          }
          else {
            FUN_0061eea0();
            FUN_00507e90();
          }
        }
      }
    }
  }
  else if (*(char *)(param_1 + 0x144) != '\0') {
    fVar10 = (float10)FUN_0043e510();
    *(float *)(param_1 + 0x140) = (float)((float10)*(float *)(param_1 + 0x140) - fVar10);
    fVar10 = (float10)FUN_0043e510();
    *(float *)(param_1 + 0x148) = (float)((float10)*(float *)(param_1 + 0x148) - fVar10);
    fVar1 = *(float *)(param_1 + 0x140);
    fVar2 = (float)_DAT_00697ce0;
    if ((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) {
      FUN_00420a60();
      iVar5 = rand();
      *(float *)(param_1 + 0x140) = (float)(iVar5 % 0xd) / (float)_DAT_00698a30;
      if (*(float *)(param_1 + 0x148) < (float)_DAT_00698f90) {
        iVar5 = rand();
        *(float *)(param_1 + 0x140) = (float)(iVar5 % 10) / (float)_DAT_00698a30;
      }
      if (*(float *)(param_1 + 0x148) < (float)_DAT_00698a30) {
        iVar5 = rand();
        *(float *)(param_1 + 0x140) = (float)(iVar5 % 7) / (float)_DAT_00698a30;
      }
      if (*(float *)(param_1 + 0x148) < (float)_DAT_00698818) {
        uVar8 = rand();
        uVar8 = uVar8 & 0x80000003;
        if ((int)uVar8 < 0) {
          uVar8 = (uVar8 - 1 | 0xfffffffc) + 1;
        }
        *(float *)(param_1 + 0x140) = (float)(int)uVar8 / (float)_DAT_00698a30;
      }
      rand();
      rand();
      pcStack_8c = "j";
      FUN_0043bc20();
      FUN_0043bc00();
      pcStack_8c = local_24;
      puStack_90 = local_58;
      uStack_94 = 0x5265f7;
      FUN_00401930();
      uStack_94 = 0x5265fe;
      pcStack_8c = (char *)FUN_004014c0();
      puStack_90 = (undefined1 *)0x2;
      uStack_94 = 0x52660c;
      FUN_0061f970();
      local_28 = rand();
      local_28 = local_28 % 100;
      if (local_28 < 0x32) {
        pcStack_8c = "BigExplosion";
        puStack_90 = (undefined1 *)0x526637;
        FUN_004026b0();
      }
    }
    if (*(char *)(param_1 + 0x14c) == '\0') {
      fVar1 = *(float *)(param_1 + 0x148);
      fVar2 = (float)_DAT_00697ce0;
      if ((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) {
        *(undefined1 *)(param_1 + 0x14c) = 1;
        piVar9 = (int *)FUN_00462a90();
        pcStack_8c = (char *)0x526684;
        (**(code **)(*piVar9 + 0xc))();
      }
    }
  }
  ExceptionList = local_10;
  return;
}

