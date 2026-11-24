// FUN_00550cf0 @ 0x550cf0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00550cf0(int *param_1)

{
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  float10 fVar5;
  float10 fVar6;
  undefined1 *puVar7;
  undefined1 *puVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  function<void___cdecl(unsigned_int_const&)> *pfVar11;
  undefined1 local_88 [8];
  void *local_80;
  undefined1 local_7c [8];
  undefined1 local_74 [8];
  undefined1 local_6c [8];
  undefined1 local_64 [8];
  undefined1 local_5c [8];
  undefined1 local_54 [8];
  undefined4 local_4c;
  undefined1 local_48 [8];
  function<void___cdecl(unsigned_int_const&)> local_40 [28];
  undefined1 local_24 [11];
  char local_19;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00638395;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar2 = FUN_00507ec0(DAT_006c1e94 ^ (uint)(local_24 + 0x20));
  if ((iVar2 != 2) &&
     (((param_1[0x96] == 0 ||
       (iVar2 = FUN_00401930(), (float)_DAT_00698c80 <= (float)param_1[0x17] - *(float *)(iVar2 + 4)
       )) || (iVar2 = FUN_00401930(),
             (float)param_1[0x17] - *(float *)(iVar2 + 4) <= (float)_DAT_00698990)))) {
    param_1[0x96] = 0;
    local_19 = '\0';
    local_4c = _DAT_00698c78;
    FUN_00550ac0(_DAT_00698c78);
    fVar5 = (float10)FUN_00550a90(local_4c);
    fVar6 = (float10)FUN_0043e510();
    FUN_00550a50((float)(fVar6 * (float10)(double)fVar5));
    uVar3 = FUN_006253a0();
    FUN_0043bc20(0,uVar3);
    uVar10 = 0;
    uVar9 = 0x800;
    puVar8 = local_24;
    puVar7 = local_5c;
    uVar3 = FUN_00401930(puVar7,puVar8,0x800,0);
    FUN_00403d90(local_54,uVar3);
    puVar4 = (undefined4 *)FUN_004014c0(puVar7,puVar8);
    iVar2 = FUN_00445ea0(*puVar4,puVar4[1],uVar9,uVar10);
    if (iVar2 != 0) {
      local_19 = '\x01';
    }
    FUN_00427c40();
    local_8 = 0;
    fVar5 = (float10)FUN_0043e510();
    FUN_0043bc10(0,(float)(fVar5 * (float10)_DAT_00698c70));
    pfVar11 = local_40;
    uVar9 = 0x80000000;
    puVar8 = local_48;
    puVar7 = local_6c;
    uVar3 = FUN_00401930(puVar7,puVar8,0x80000000,pfVar11);
    FUN_00403d90(local_64,uVar3);
    puVar4 = (undefined4 *)FUN_004014c0(puVar7,puVar8);
    iVar2 = FUN_00445ea0(*puVar4,puVar4[1],uVar9,pfVar11);
    if (iVar2 == 0) {
      _String_const_iterator<>();
      puVar4 = (undefined4 *)FUN_00472660(local_74);
      local_18 = *puVar4;
      local_14 = puVar4[1];
      uVar3 = FUN_0046df60(local_7c);
      cVar1 = FUN_00426920(uVar3);
      if (cVar1 != '\0') {
        puVar4 = (undefined4 *)FUN_004bdc80();
        local_80 = (void *)*puVar4;
        operator_delete(local_80);
                    /* WARNING: Subroutine does not return */
        FUN_0042f830();
      }
      if (local_19 == '\0') {
        (**(code **)(*param_1 + 0x10))(param_1 + 0x56);
        fVar5 = (float10)FUN_0043e510();
        param_1[0x94] = (int)(float)(fVar5 + (float10)(float)param_1[0x94]);
      }
      else {
        uVar3 = FUN_004014c0(local_88,local_48);
        FUN_00401510(uVar3);
        if ((int *)param_1[0x1d] != param_1 + 0x75) {
          (**(code **)(*param_1 + 0x10))(param_1 + 0x75);
        }
      }
      if ((float)_DAT_00698c68 < (float)param_1[0x94]) {
        FUN_004405c0("ReturnToOrigin",param_1 + 1);
      }
      local_8 = 0xffffffff;
      std::function<void___cdecl(unsigned_int_const&)>::~function<void___cdecl(unsigned_int_const&)>
                (local_40);
    }
    else {
      FUN_00427cc0();
      uVar3 = FUN_0043ed20();
      FUN_00550bd0(uVar3);
      FUN_00427cc0();
      iVar2 = FUN_0043ed20();
      param_1[0x96] = iVar2;
      FUN_004405c0("MoveAnchored",param_1 + 1);
      local_8 = 0xffffffff;
      std::function<void___cdecl(unsigned_int_const&)>::~function<void___cdecl(unsigned_int_const&)>
                (local_40);
    }
  }
  ExceptionList = local_10;
  return;
}

