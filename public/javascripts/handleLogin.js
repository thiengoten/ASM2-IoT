window.addEventListener("load", function () {
  var accounts = [
    {
      username: "user02",
      password: "1234",
    },
    {
      username: "user03",
      password: "12345",
    },
    {
      username: "user04",
      password: "123456",
    },
  ];
  // localStorage.setItem("accounts", JSON.stringify(accounts));

  function getAccount() {
    // debugger;
    var username = document.getElementById("username").value;
    console.log(
      "🚀 ~ file: handleLogin.js ~ line 24 ~ getAccount ~ username",
      username
    );
    var password = document.getElementById("password").value;
    console.log(
      "🚀 ~ file: handleLogin.js ~ line 26 ~ getAccount ~ password",
      password
    );

    console.log("Username: " + username + " Password: " + password);

    for (i = 0; i < accounts.length; i++) {
      if (
        username == accounts[i].username &&
        password == accounts[i].password
      ) {
        console.log(username + "is logged in!!!");
        localStorage.setItem("username", JSON.stringify(username));
        window.location.href = "http://localhost:3000";
        alert(username + " is login successful!!!");
        return;
      }
      // else {
      //   console.log("Incorret username or password");
      //   alert("Incorrect username or password");
      // }
    }
    console.log("Incorret username or password");
  }

  document.getElementById("loginForm").addEventListener("submit", function (e) {
    e.preventDefault();
    console.log("hello");

    getAccount();
    console.log("hi");
  });
});