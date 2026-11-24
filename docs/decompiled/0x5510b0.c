// FUN_005510b0 @ 0x5510b0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_005510b0(int *param_1)

{
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  int *piVar5;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *this;
  char *_Str1;
  float10 fVar6;
  float10 fVar7;
  undefined1 *puVar8;
  undefined1 *puVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  function<void___cdecl(unsigned_int_const&)> *pfVar12;
  char *_Str2;
  undefined1 local_d4 [8];
  undefined1 local_cc [8];
  undefined1 local_c4 [8];
  void *local_bc;
  undefined1 local_b8 [8];
  undefined1 local_b0 [8];
  undefined1 local_a8 [8];
  undefined1 local_a0 [8];
  undefined1 local_98 [8];
  undefined1 local_90 [8];
  undefined1 local_88 [8];
  undefined1 *local_80;
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined1 local_70 [8];
  undefined4 local_68;
  int local_64;
  undefined4 local_60;
  function<void___cdecl(unsigned_int_const&)> local_5c [28];
  undefined1 local_40 [8];
  function<void___cdecl(unsigned_int_const&)> local_38 [31];
  char local_19;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_006383c3;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar2 = FUN_00507ec0(DAT_006c1e94 ^ (uint)(local_38 + 0x34));
  if (iVar2 != 2) {
    local_19 = '\0';
    FUN_00427c40();
    local_8 = 0;
    FUN_00427c40();
    local_8 = CONCAT31(local_8._1_3_,1);
    _String_const_iterator<>();
    local_60 = _DAT_00698178;
    FUN_00550ac0(_DAT_00698178);
    fVar6 = (float10)FUN_00550a90(local_60);
    fVar7 = (float10)FUN_0043e510();
    FUN_00550a50((float)(fVar7 * (float10)(double)fVar6));
    uVar3 = FUN_006253a0();
    FUN_0043bc20(0,uVar3);
    uVar11 = 0;
    uVar10 = 0x800;
    puVar9 = local_40;
    puVar8 = local_90;
    uVar3 = FUN_00401930(puVar8,puVar9,0x800,0);
    FUN_00403d90(local_88,uVar3);
    puVar4 = (undefined4 *)FUN_004014c0(puVar8,puVar9);
    iVar2 = FUN_00445ea0(*puVar4,puVar4[1],uVar10,uVar11);
    if (iVar2 != 0) {
      local_19 = '\x01';
    }
    pfVar12 = local_5c;
    uVar10 = 0x80000000;
    uVar3 = FUN_00401930(0x80000000,pfVar12);
    puVar4 = (undefined4 *)FUN_00403d90(local_98,uVar3);
    FUN_00445ea0(*puVar4,puVar4[1],uVar10,pfVar12);
    puVar4 = (undefined4 *)FUN_00472660(local_a0);
    local_18 = *puVar4;
    local_14 = puVar4[1];
    uVar3 = FUN_0046df60(local_a8);
    cVar1 = FUN_00426920(uVar3);
    if (cVar1 != '\0') {
      piVar5 = (int *)FUN_004bdc80();
      local_64 = *piVar5;
      if (((*(uint *)(local_64 + 8) & 0x80000000) != 0) &&
         (local_68 = FUN_0043ed20(), *(float *)(local_64 + 4) < (float)_DAT_00698820)) {
        _Str2 = "Fall";
        this = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_0043d890();
        _Str1 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(this);
        iVar2 = strcmp(_Str1,_Str2);
        if (iVar2 != 0) {
          FUN_00433600(&local_68);
        }
      }
                    /* WARNING: Subroutine does not return */
      FUN_0042f830();
    }
    puVar4 = (undefined4 *)FUN_00472660(local_b0);
    local_18 = *puVar4;
    local_14 = puVar4[1];
    uVar3 = FUN_0046df60(local_b8);
    cVar1 = FUN_00426920(uVar3);
    if (cVar1 != '\0') {
      puVar4 = (undefined4 *)FUN_004bdc80();
      local_bc = (void *)*puVar4;
      operator_delete(local_bc);
                    /* WARNING: Subroutine does not return */
      FUN_0042f830();
    }
    iVar2 = FUN_004ac790();
    if (iVar2 == 0) {
      FUN_004405c0(&DAT_00676430,param_1 + 1);
    }
    else {
      FUN_00427ce0();
      if (local_19 == '\0') {
        (**(code **)(*param_1 + 0x10))(param_1 + 0x56);
      }
      else {
        if ((int *)param_1[0x1d] != param_1 + 0x75) {
          (**(code **)(*param_1 + 0x10))(param_1 + 0x75);
        }
        fVar6 = (float10)FUN_0043e510();
        FUN_0043bc10(0,(float)(fVar6 * (float10)_DAT_00698340));
        puVar4 = (undefined4 *)FUN_00472660(local_c4);
        local_78 = *puVar4;
        local_74 = puVar4[1];
        while( true ) {
          uVar3 = FUN_0046df60(local_cc);
          cVar1 = FUN_00426920(uVar3);
          if (cVar1 == '\0') break;
          local_80 = local_70;
          local_7c = FUN_0041efa0();
          puVar4 = (undefined4 *)FUN_004bdc80();
          (**(code **)(*(int *)*puVar4 + 0xc))(7,&local_80);
          FUN_0042f8a0(local_d4,0);
        }
        puVar4 = (undefined4 *)FUN_00427cc0();
        FUN_00550bd0(*puVar4);
      }
    }
    local_8 = local_8 & 0xffffff00;
    std::function<void___cdecl(unsigned_int_const&)>::~function<void___cdecl(unsigned_int_const&)>
              (local_5c);
    local_8 = 0xffffffff;
    std::function<void___cdecl(unsigned_int_const&)>::~function<void___cdecl(unsigned_int_const&)>
              (local_38);
  }
  ExceptionList = local_10;
  return;
}

