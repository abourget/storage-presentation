pragma solidity >=0.4.0 <0.7.0;

contract Main {
  public string var1; // 0000000000000000000000000000000000000000000000000000000000000000
  uint256 var2;

  mapping(address => uint256) balances;
  []address users;

  function storeThings() public {
    var1 = "hello";
    var2 = "hello this is a very long string where we have more than 32 characters";

    balances[msg.sender] = 234;
    balances[0x123456] = 234;
    balances[0x3245] = 234;

    users.push(msg.sender);  //
  }

  event Transfer(address indexed from, address indexed to, uint256 amount);

  function muchLogs() public payable {
    // TODO: transfer money here
    emit Transfer(msg.sender, msg.sender, 123);
  }
}
