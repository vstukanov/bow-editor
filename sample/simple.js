// ES6 syntax
function Person(){
  this.age = 0;
  this.text = "Hello, World!";

  setInterval(() => {
    // `this` now refers to the Person object, brilliant!
    this.age++;
  }, 1000);``

  return 200;
}

const foo = {
  my_band: (ev) => {
    console.log(ev, true);
  },
};

foo.my_band();

var p = new Person();
