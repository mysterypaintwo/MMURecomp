// FUN_004fc280 @ 0x4fc280

void __fastcall FUN_004fc280(int param_1)

{
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  undefined1 *puVar4;
  undefined1 local_1c [16];
  int local_c;
  int local_8;
  
  cVar1 = FUN_00443240();
  if ((cVar1 == '\0') && (*(char *)(param_1 + 0x1008) != '\0')) {
    FUN_004026b0("GameOverLoop",1,1);
  }
  cVar1 = FUN_00441d90();
  if (cVar1 != '\0') {
    do {
      *(int *)(param_1 + 0xfcc) = *(int *)(param_1 + 0xfcc) + 1;
      FUN_004026b0("Select",0,0);
      if (0x11 < *(int *)(param_1 + 0xfcc)) {
        *(undefined4 *)(param_1 + 0xfcc) = 0;
      }
      iVar2 = FUN_00402b30(*(undefined4 *)(param_1 + 0xfcc));
    } while (*(int *)(iVar2 + 0x1c) == 0);
  }
  cVar1 = FUN_00441d90();
  if (cVar1 != '\0') {
    do {
      *(int *)(param_1 + 0xfcc) = *(int *)(param_1 + 0xfcc) + -1;
      FUN_004026b0("Select",0,0);
      if (*(int *)(param_1 + 0xfcc) < 0) {
        *(undefined4 *)(param_1 + 0xfcc) = 0x11;
      }
      iVar2 = FUN_00402b30(*(undefined4 *)(param_1 + 0xfcc));
    } while (*(int *)(iVar2 + 0x1c) == 0);
  }
  cVar1 = FUN_00441d90();
  if ((cVar1 != '\0') || (cVar1 = FUN_00441d90(), cVar1 != '\0')) {
    iVar2 = FUN_00402b30(*(undefined4 *)(param_1 + 0xfcc));
    *(undefined4 *)(param_1 + 0xfd0) = *(undefined4 *)(iVar2 + 0x1c);
    switch(*(undefined4 *)(param_1 + 0xfd0)) {
    case 0x38:
      local_8 = FUN_004bd9f0();
      iVar2 = FUN_004bd9f0();
      cVar1 = FUN_00441d90();
      local_c = iVar2 + ((-(uint)(cVar1 != '\0') & 2) - 1);
      local_c = FUN_004fb010(local_c,1,5);
      FUN_004f0ba0(local_c);
      FUN_004fb0e0();
      if (local_8 == local_c) {
        return;
      }
      FUN_004026b0("Select",0,0);
      return;
    case 0x3b:
      FUN_004026b0("Select",0,0);
      cVar1 = FUN_004054e0();
      FUN_00431960(cVar1 == '\0');
      FUN_004fb0e0();
      return;
    case 0x3c:
      FUN_004026b0("Select",0,0);
      cVar1 = FUN_00420f50();
      FUN_00431980(cVar1 == '\0');
      FUN_004fb0e0();
      return;
    case 0x3e:
      FUN_004026b0("Select",0,0);
      cVar1 = FUN_004042e0();
      FUN_004319a0(cVar1 == '\0');
      FUN_004fb0e0();
      return;
    case 0x3f:
      FUN_004026b0("Select",0,0);
      cVar1 = FUN_00425e60();
      FUN_004319e0(cVar1 == '\0');
      FUN_004fb0e0();
      return;
    case 0x40:
      FUN_004026b0("Select",0,0);
      cVar1 = FUN_00428b10();
      FUN_004319c0(cVar1 == '\0');
      FUN_004fb0e0();
      return;
    }
  }
  cVar1 = FUN_00441d90();
  if (cVar1 == '\0') {
LAB_004fc723:
    uVar3 = FUN_0043bc20(0x28,*(int *)(param_1 + 0xfcc) * 8 + 0x28);
    puVar4 = local_1c;
    FUN_00401930(puVar4,uVar3);
    uVar3 = FUN_004014c0(puVar4,uVar3);
    FUN_004026e0(uVar3);
  }
  else {
    iVar2 = FUN_00402b30(*(undefined4 *)(param_1 + 0xfcc));
    *(undefined4 *)(param_1 + 0xfd0) = *(undefined4 *)(iVar2 + 0x1c);
    switch(*(undefined4 *)(param_1 + 0xfd0)) {
    case 0x38:
    case 0x3b:
    case 0x3c:
    case 0x3e:
    case 0x3f:
    case 0x40:
      break;
    case 0x39:
      FUN_004405c0("MapKeyboardInputs",param_1 + 4);
      break;
    case 0x3a:
      FUN_004405c0("MapJoystickInputs",param_1 + 4);
      break;
    case 0x3d:
      FUN_004fbb10(DAT_006c55fc);
      FUN_00431960(0);
      FUN_00431980(1);
      FUN_004319a0(1);
      FUN_004319c0(1);
      FUN_004319e0(0);
      FUN_004026b0("Confirm",0,0);
      FUN_004fb0e0();
      break;
    case 0x41:
      FUN_004f0d90();
    default:
      if (*(int *)(param_1 + 0xfd0) != 0) {
        FUN_004026b0("Confirm",0,0);
        FUN_004405c0("Flash",param_1 + 4);
      }
      goto LAB_004fc723;
    }
  }
  return;
}

