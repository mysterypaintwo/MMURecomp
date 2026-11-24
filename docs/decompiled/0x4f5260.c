// FUN_004f5260 @ 0x4f5260

void __fastcall FUN_004f5260(int param_1)

{
  char cVar1;
  undefined4 uVar2;
  uint uVar3;
  int iVar4;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_d0 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_b4 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_98 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_7c [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_60 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_44 [46];
  char local_16;
  char local_15;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00637fad;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004f4120(DAT_006c1e94 ^ (uint)(local_44 + 0x40));
  uVar2 = FUN_0043bc20(*(int *)(param_1 + 0x1700) * 0x28 + 0x5b,
                       *(int *)(param_1 + 0x1704) * 0x30 + 0xb);
  FUN_004026e0(uVar2);
  uVar2 = FUN_0043bc20(0xb8,0x68);
  FUN_004026e0(uVar2);
  uVar3 = (uint)(*(int *)(param_1 + 0x1704) != -1);
  FUN_00403240(uVar3);
  FUN_00403220(uVar3);
  uVar3 = (uint)(*(int *)(param_1 + 0x1704) == -1);
  FUN_00403240(uVar3);
  FUN_00403220(uVar3);
  local_14 = 0;
  local_15 = FUN_004f3c40(&local_14);
  if (local_15 == '\0') {
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_98,"");
    local_8 = 3;
    FUN_004f3ab0(local_98);
    local_8 = 0xffffffff;
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_98);
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_b4,"");
    local_8 = 4;
    FUN_004f3ab0(local_b4);
    local_8 = 0xffffffff;
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_b4);
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_d0,"");
    local_8 = 5;
    FUN_004f3ab0(local_d0);
    local_8 = 0xffffffff;
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_d0);
  }
  else {
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    basic_string<char,std::char_traits<char>,std::allocator<char>_>
              (local_44,(&PTR_s_1_UP__006c2030)[local_14 * 3]);
    local_8 = 0;
    FUN_004f3ab0(local_44);
    local_8 = 0xffffffff;
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_44);
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    basic_string<char,std::char_traits<char>,std::allocator<char>_>
              (local_60,(&PTR_s_GRANTS_YOU_AN_EXTRA_LIFE__006c2034)[local_14 * 3]);
    local_8 = 1;
    FUN_004f3ab0(local_60);
    local_8 = 0xffffffff;
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_60);
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    basic_string<char,std::char_traits<char>,std::allocator<char>_>
              (local_7c,(&PTR_DAT_006c2038)[local_14 * 3]);
    local_8 = 2;
    FUN_004f3ab0(local_7c);
    local_8 = 0xffffffff;
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_7c);
  }
  cVar1 = FUN_00441d90();
  if (cVar1 != '\0') {
    if (*(int *)(param_1 + 0x1704) == -1) {
      FUN_004026b0("Confirm",0,0);
      FUN_004405c0("SlideOut",param_1 + 4);
      ExceptionList = local_10;
      return;
    }
    if (local_15 != '\0') {
      iVar4 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
      if (iVar4 < *(int *)(param_1 + 0x16e0 + local_14 * 4)) {
        FUN_004405c0("NotEnoughBolts",param_1 + 4);
        ExceptionList = local_10;
        return;
      }
      local_16 = '\0';
      switch(local_14) {
      case 0:
        iVar4 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
        if (8 < iVar4) {
          FUN_004405c0("TooManyItems",param_1 + 4);
          ExceptionList = local_10;
          return;
        }
        local_16 = '\x01';
        break;
      case 1:
        iVar4 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
        if (3 < iVar4) {
          FUN_004405c0("TooManyItems",param_1 + 4);
          ExceptionList = local_10;
          return;
        }
        local_16 = '\x01';
        break;
      case 2:
        iVar4 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
        if (3 < iVar4) {
          FUN_004405c0("TooManyItems",param_1 + 4);
          ExceptionList = local_10;
          return;
        }
        local_16 = '\x01';
        break;
      case 3:
        iVar4 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
        if (3 < iVar4) {
          FUN_004405c0("TooManyItems",param_1 + 4);
          ExceptionList = local_10;
          return;
        }
        local_16 = '\x01';
        break;
      case 4:
        iVar4 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
        if (8 < iVar4) {
          FUN_004405c0("TooManyItems",param_1 + 4);
          ExceptionList = local_10;
          return;
        }
        local_16 = '\x01';
        break;
      case 5:
        iVar4 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
        if (8 < iVar4) {
          FUN_004405c0("TooManyItems",param_1 + 4);
          ExceptionList = local_10;
          return;
        }
        local_16 = '\x01';
        break;
      case 6:
        iVar4 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
        if (8 < iVar4) {
          FUN_004405c0("TooManyItems",param_1 + 4);
          ExceptionList = local_10;
          return;
        }
        local_16 = '\x01';
        break;
      case 7:
        iVar4 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
        if (0 < iVar4) {
          FUN_004405c0("TooManyItems",param_1 + 4);
          ExceptionList = local_10;
          return;
        }
        local_16 = '\x01';
      }
      if (local_16 == '\0') {
        ExceptionList = local_10;
        return;
      }
      FUN_004405c0("Confirm",param_1 + 4);
      ExceptionList = local_10;
      return;
    }
  }
  FUN_004f3cc0(param_1 + 0xdf0,*(undefined4 *)(param_1 + 0x16e8),2);
  FUN_004f3cc0(param_1 + 0xf40,*(undefined4 *)(param_1 + 0x16ec),2);
  FUN_004f3cc0(param_1 + 0xca0,*(undefined4 *)(param_1 + 0x16e4),2);
  FUN_004f3cc0(param_1 + 0xb50,*(undefined4 *)(param_1 + 0x16e0),2);
  FUN_004f3cc0(param_1 + 0x610,*(undefined4 *)(param_1 + 0x16f0),2);
  FUN_004f3cc0(param_1 + 0x760,*(undefined4 *)(param_1 + 0x16f4),2);
  FUN_004f3cc0(param_1 + 0x8b0,*(undefined4 *)(param_1 + 0x16f8),2);
  FUN_004f3cc0(param_1 + 0xa00,*(undefined4 *)(param_1 + 0x16fc),2);
  uVar2 = 3;
  iVar4 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
  FUN_004f3cc0(param_1 + 0x4c0,iVar4,uVar2);
  ExceptionList = local_10;
  return;
}

