// FUN_0054d350 @ 0x54d350

void __fastcall FUN_0054d350(int param_1)

{
  char cVar1;
  int iVar2;
  char *_Str;
  
  iVar2 = FUN_00507ec0();
  if ((iVar2 != 2) && (iVar2 = FUN_00507ec0(), iVar2 != 4)) {
    if (*(char *)(param_1 + 0x154) == '\0') {
      _Str = (char *)FUN_00424ff0();
      iVar2 = atoi(_Str);
      *(char *)(param_1 + 0x155) = '\x01' - (iVar2 != 1);
    }
    *(undefined1 *)(param_1 + 0x154) = 1;
    cVar1 = FUN_0054d150();
    if (cVar1 != '\0') {
      FUN_004405c0("Shoot",param_1 + 4);
    }
  }
  return;
}

