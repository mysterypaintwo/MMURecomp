// FUN_005afce0 @ 0x5afce0

void __fastcall FUN_005afce0(int param_1)

{
  bool bVar1;
  bool bVar2;
  char cVar3;
  undefined4 uVar4;
  
  cVar3 = FUN_005a82b0();
  if (cVar3 == '\0') {
    if ((*(char *)(param_1 + 0x3dc) != '\0') && (cVar3 = FUN_00441d90(), cVar3 != '\0')) {
      uVar4 = SDL_GetTicks();
      *(undefined4 *)(param_1 + 0x16c) = uVar4;
    }
    if ((*(uint *)(param_1 + 0x98) & 8) == 0) {
      cVar3 = FUN_00441db0();
      if ((cVar3 == '\0') &&
         ((*(char *)(param_1 + 0x1cc) == '\0' || (*(char *)(param_1 + 0x68) != '\0')))) {
        bVar1 = false;
      }
      else {
        bVar1 = true;
      }
      cVar3 = FUN_00441db0();
      if ((cVar3 == '\0') &&
         ((*(char *)(param_1 + 0x1cc) == '\0' || (*(char *)(param_1 + 0x68) == '\0')))) {
        bVar2 = false;
      }
      else {
        bVar2 = true;
      }
      cVar3 = FUN_00405360();
      if ((cVar3 == '\0') || ((!bVar1 && (!bVar2)))) {
        if ((!bVar1) && (!bVar2)) {
          FUN_004405c0("Stand",param_1 + 4);
        }
      }
      else {
        FUN_004405c0(&DAT_0067da3c,param_1 + 4);
      }
      if ((*(uint *)(param_1 + 0x98) & 0x10) != 0) {
        FUN_004405c0(&DAT_0067da48,param_1 + 4);
      }
      cVar3 = FUN_005aa080();
      if (cVar3 == '\0') {
        FUN_005ad160();
        cVar3 = FUN_005a9de0();
        if (((cVar3 == '\0') && (cVar3 = FUN_005a9c40(1), cVar3 == '\0')) &&
           (cVar3 = FUN_005a97d0(1), cVar3 == '\0')) {
          FUN_005a87c0();
          FUN_005a8730();
        }
      }
    }
    else {
      FUN_004405c0("Stand",param_1 + 4);
    }
  }
  else {
    FUN_004405c0("Stand",param_1 + 4);
  }
  return;
}

