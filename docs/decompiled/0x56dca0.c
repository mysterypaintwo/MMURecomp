// FUN_0056dca0 @ 0x56dca0

void __thiscall FUN_0056dca0(int *param_1,int param_2,undefined4 param_3)

{
  undefined4 uVar1;
  
  if (param_2 == 0) {
    FUN_00575f40(0,param_3);
    if (param_1[0x54] == 0) {
      FUN_00562450(1);
      uVar1 = (**(code **)(*param_1 + 4))("Exploding");
      (**(code **)(*param_1 + 0x10))(uVar1);
    }
  }
  else if (param_2 == 0xf) {
    if (param_1[0xda] == 0) {
      FUN_004405c0("EnergyFill",param_1 + 1);
    }
    else if (param_1[0xda] == 1) {
      FUN_004405c0("Crush",param_1 + 1);
    }
  }
  else {
    FUN_00575f40(param_2,param_3);
  }
  return;
}

