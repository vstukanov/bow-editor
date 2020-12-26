// ES6 syntax
function Person(){
  this.age = 0;
  this.text = "Hello, World!";

  setInterval(() => {
    // `this` now refers to the Person object, brilliant!
    this.age++;
  }, 1000);
}

var p = new Person();
