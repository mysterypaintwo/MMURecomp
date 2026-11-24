// FUN_0054f0d0 @ 0x54f0d0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0054f0d0(int param_1)

{
  char cVar1;
  int iVar2;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *this;
  char *_Str1;
  undefined4 *puVar3;
  undefined4 uVar4;
  int *piVar5;
  float *pfVar6;
  float10 fVar7;
  char *_Str2;
  undefined4 uVar8;
  undefined1 local_30 [8];
  undefined1 local_28 [8];
  int local_20;
  undefined4 local_1c;
  undefined4 local_18;
  int local_14;
  undefined4 local_10;
  undefined4 local_c;
  int local_8;
  
  iVar2 = FUN_00507ec0();
  if (iVar2 != 2) {
    if (*(int *)(param_1 + 0x150) != 0) {
      _Str2 = "Death";
      this = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_0043d890();
      _Str1 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(this);
      iVar2 = strcmp(_Str1,_Str2);
      if (iVar2 == 0) {
        *(undefined4 *)(param_1 + 0x150) = 0;
      }
    }
    local_10 = *(undefined4 *)(param_1 + 0x58);
    local_c = *(undefined4 *)(param_1 + 0x5c);
    if ((float)_DAT_00697ce0 <= *(float *)(param_1 + 100)) {
      _String_const_iterator<>();
      puVar3 = (undefined4 *)FUN_00472660(local_28);
      local_1c = *puVar3;
      local_18 = puVar3[1];
      uVar4 = FUN_0046df60(local_30);
      cVar1 = FUN_00426920(uVar4);
      if (cVar1 != '\0') {
        piVar5 = (int *)FUN_004bdc80();
        local_20 = *piVar5;
        if (*(int *)(local_20 + 8) != 0x200000) {
                    /* WARNING: Subroutine does not return */
          FUN_0042f830();
        }
        FUN_004405c0(&DAT_0067612c,param_1 + 4);
        return;
      }
    }
    uVar8 = 0;
    uVar4 = 1;
    fVar7 = (float10)FUN_0043c840(1,0);
    FUN_0043d200((float)fVar7,uVar4,uVar8);
    FUN_00425d50(&local_8,&local_14);
    if (*(int *)(param_1 + 0x150) == 0) {
      iVar2 = FUN_00420780();
      FUN_00551c10(param_1,&local_10,(float)local_14,(float)iVar2);
    }
    else {
      pfVar6 = (float *)FUN_00401930();
      if ((*pfVar6 + (float)_DAT_00698ad8 < *(float *)(param_1 + 0x58) - (float)local_8) ||
         (pfVar6 = (float *)FUN_00401930(),
         (float)local_8 + *(float *)(param_1 + 0x58) < *pfVar6 - (float)_DAT_00698ad8)) {
        uVar4 = 0;
        puVar3 = (undefined4 *)FUN_00420a40(0);
        uVar4 = FUN_0043bc10(*puVar3,uVar4);
        FUN_00420160(uVar4);
        (**(code **)(**(int **)(param_1 + 0x150) + 0xc))(9,0);
        *(undefined4 *)(param_1 + 0x150) = 0;
      }
      else {
        FUN_00551b60(param_1,*(undefined4 *)(param_1 + 0x150),(float)local_14,_DAT_006760bc);
      }
    }
  }
  return;
}

