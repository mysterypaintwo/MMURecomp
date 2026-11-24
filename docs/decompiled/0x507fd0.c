// FUN_00507fd0 @ 0x507fd0

void __thiscall FUN_00507fd0(int param_1,char param_2)

{
  undefined4 uVar1;
  
  if (DAT_006c55fc != 0) {
    FUN_00441b70(1);
  }
  if ((param_2 != '\0') && (DAT_006c55e8 != 0)) {
    Mix_HaltMusic();
  }
  if (DAT_006c55f8 != 0) {
    uVar1 = FUN_0043bc20(0,0);
    FUN_00403ef0(uVar1);
  }
  if (*(int *)(param_1 + 0x20) != 0) {
    *(undefined1 *)(param_1 + 0x5e) = 0;
    if (*(undefined4 **)(param_1 + 0x20) != (undefined4 *)0x0) {
      (**(code **)**(undefined4 **)(param_1 + 0x20))(1);
    }
    *(undefined4 *)(param_1 + 0x20) = 0;
  }
  DAT_006c55f0 = 0;
  if (*(int *)(param_1 + 0x18) != 0) {
    if (*(undefined4 **)(param_1 + 0x18) != (undefined4 *)0x0) {
      (**(code **)**(undefined4 **)(param_1 + 0x18))(1);
    }
    *(undefined4 *)(param_1 + 0x18) = 0;
  }
  if (*(int *)(param_1 + 0x1c) != 0) {
    if (*(undefined4 **)(param_1 + 0x1c) != (undefined4 *)0x0) {
      (**(code **)**(undefined4 **)(param_1 + 0x1c))(1);
    }
    *(undefined4 *)(param_1 + 0x1c) = 0;
  }
  if (*(int *)(param_1 + 0x14) != 0) {
    if (*(undefined4 **)(param_1 + 0x14) != (undefined4 *)0x0) {
      (**(code **)**(undefined4 **)(param_1 + 0x14))(1);
    }
    *(undefined4 *)(param_1 + 0x14) = 0;
  }
  return;
}

