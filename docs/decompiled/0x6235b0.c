// FUN_006235b0 @ 0x6235b0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_006235b0(int param_1)

{
  char cVar1;
  uint uVar2;
  undefined4 uVar3;
  int iVar4;
  int extraout_EAX;
  int extraout_EAX_00;
  undefined4 *puVar5;
  float10 fVar6;
  undefined1 local_80 [8];
  undefined1 local_78 [8];
  undefined4 *local_70;
  undefined4 *local_6c;
  undefined1 local_68 [8];
  undefined1 local_60 [8];
  undefined1 local_58 [8];
  undefined1 local_50 [8];
  undefined4 local_48;
  undefined4 local_44;
  undefined1 local_40 [8];
  undefined1 local_38 [8];
  undefined1 local_30 [8];
  undefined1 local_28 [8];
  undefined1 local_20 [8];
  int local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined1 local_c [8];
  
  cVar1 = FUN_004ad6a0();
  if (cVar1 == '\0') {
    if ((((DAT_006c5758 == '\0') && (*(char *)(param_1 + 0xd4) == '\0')) ||
        (uVar2 = FUN_00507ec0(), (uVar2 & 0x80) == 0)) ||
       (((cVar1 = FUN_00441d90(), cVar1 == '\0' && (cVar1 = FUN_00441d90(), cVar1 == '\0')) &&
        (cVar1 = FUN_00441d90(), cVar1 == '\0')))) {
      cVar1 = FUN_00441d90();
      if (cVar1 == '\0') {
        cVar1 = FUN_00441d90();
        if ((cVar1 != '\0') && ((float)_DAT_00697ce8 < *(float *)(param_1 + 0x3c))) {
          *(float *)(param_1 + 0x3c) = *(float *)(param_1 + 0x3c) - (float)_DAT_00697ce8;
        }
      }
      else {
        *(float *)(param_1 + 0x3c) = *(float *)(param_1 + 0x3c) + (float)_DAT_00697ce8;
      }
      cVar1 = FUN_00428ca0();
      if (cVar1 != '\0') {
        fVar6 = (float10)FUN_0043e510();
        *(float *)(param_1 + 0x38) =
             (float)(fVar6 * (float10)*(float *)(param_1 + 0x3c) +
                    (float10)*(float *)(param_1 + 0x38));
      }
      cVar1 = FUN_00428ad0();
      if (((cVar1 != '\0') && ((float)_DAT_00697ce8 < *(float *)(param_1 + 0x38))) &&
         (uVar2 = FUN_00507ec0(), (uVar2 & 0x80) != 0)) {
        FUN_00441b70(0);
      }
      cVar1 = FUN_00428ca0();
      if (cVar1 != '\0') {
        cVar1 = FUN_00405360();
        if (cVar1 == '\0') {
          if ((*(int *)(param_1 + 0x48) != 0) &&
             ((float)*(int *)(param_1 + 0x54) <= *(float *)(param_1 + 0x38) * (float)_DAT_00698798))
          {
            cVar1 = FUN_006228a0();
            if (cVar1 == '\0') {
              return;
            }
            cVar1 = FUN_004ad6a0();
            if (cVar1 != '\0') {
              return;
            }
            uVar3 = FUN_0042d980();
            *(undefined4 *)(param_1 + 0x48) = uVar3;
            *(undefined4 *)(param_1 + 0x54) = 0;
            if (*(int *)(param_1 + 0x48) == 0) {
              return;
            }
            iVar4 = FUN_004481a0(param_1 + 0x54);
            if (iVar4 != 0) {
              return;
            }
          }
        }
        else {
          CDHtmlElementEventSink::~CDHtmlElementEventSink
                    (*(CDHtmlElementEventSink **)(param_1 + 0x4c));
          if ((*(int *)(param_1 + 0x50) < extraout_EAX) &&
             ((float)*(int *)(param_1 + 0x54) <= *(float *)(param_1 + 0x38) * (float)_DAT_00698798))
          {
            cVar1 = FUN_00621e00();
            if (cVar1 == '\0') {
              return;
            }
            cVar1 = FUN_004ad6a0();
            if (cVar1 != '\0') {
              return;
            }
            *(int *)(param_1 + 0x50) = *(int *)(param_1 + 0x50) + 1;
            FUN_00433b80(*(undefined4 *)(param_1 + 0x50));
            uVar3 = FUN_004ac790();
            *(undefined4 *)(param_1 + 0x54) = uVar3;
            CDHtmlElementEventSink::~CDHtmlElementEventSink
                      (*(CDHtmlElementEventSink **)(param_1 + 0x4c));
            if (extraout_EAX_00 <= *(int *)(param_1 + 0x50)) {
              return;
            }
          }
        }
      }
      _String_const_iterator<>();
      local_18 = -1;
      while (local_18 < 9) {
        local_18 = local_18 + 1;
        puVar5 = (undefined4 *)FUN_004b7660(local_28);
        local_14 = *puVar5;
        local_10 = puVar5[1];
        while( true ) {
          uVar3 = FUN_0046a130(local_30);
          cVar1 = FUN_0046bcb0(uVar3);
          if (cVar1 == '\0') break;
          FUN_00432750();
          iVar4 = FUN_00433970();
          if ((iVar4 == local_18) && (uVar2 = FUN_00507ec0(), (uVar2 & 0x80) != 0)) {
            iVar4 = FUN_00432750();
            (**(code **)(**(int **)(iVar4 + 4) + 0x2c))();
            iVar4 = FUN_00432750();
            (**(code **)(**(int **)(iVar4 + 4) + 0x3c))();
          }
          FUN_00433d90();
        }
      }
      local_18 = local_18 + 1;
      FUN_004b7660(local_c);
      while( true ) {
        uVar3 = FUN_0046a130(local_38);
        cVar1 = FUN_0046bcb0(uVar3);
        if (cVar1 == '\0') break;
        uVar3 = FUN_004320e0();
        FUN_00433cd0(local_20,uVar3);
        uVar3 = FUN_0046a130(local_40);
        cVar1 = FUN_0046bcb0(uVar3);
        if (cVar1 == '\0') {
          FUN_0042f900();
        }
        else {
          FUN_00433d70();
          cVar1 = FUN_00433a50();
          if (cVar1 == '\0') {
            FUN_0042f900();
          }
          else {
            puVar5 = (undefined4 *)FUN_0042f920(local_50,0);
            local_48 = *puVar5;
            local_44 = puVar5[1];
            FUN_0042f050(local_58,local_48,local_44);
          }
        }
      }
      puVar5 = (undefined4 *)FUN_004b7660(local_60);
      local_14 = *puVar5;
      local_10 = puVar5[1];
      while( true ) {
        uVar3 = FUN_0046a130(local_68);
        cVar1 = FUN_0046bcb0(uVar3);
        if (cVar1 == '\0') break;
        FUN_004320e0();
        cVar1 = FUN_004047f0();
        if (cVar1 == '\0') {
          FUN_00433d90();
        }
        else {
          iVar4 = FUN_004320e0();
          local_70 = *(undefined4 **)(iVar4 + 4);
          local_6c = local_70;
          if (local_70 != (undefined4 *)0x0) {
            (**(code **)*local_70)(1);
          }
          puVar5 = (undefined4 *)FUN_00429950(local_78,0);
          FUN_00429110(local_80,*puVar5,puVar5[1]);
        }
      }
    }
    else {
      FUN_00621b10();
      *(undefined1 *)(param_1 + 0x34) = 1;
      FUN_00511030();
    }
  }
  return;
}

