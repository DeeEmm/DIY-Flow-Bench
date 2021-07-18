var ParallelRunner = require("../index").ParallelRunner

var should = require("should")

describe("When running tasks parallely", function() {
  it("", function(done) {
    var r = new ParallelRunner()

    var list = []
    var numCallback = 10

    var func = function(i, next) {

      setTimeout(function() {

        list[i] = i
        next(i)

      }, 20-i)
    }

    for(var i = 0 ; i < numCallback ; i++) {

      r.add(func, i)

    }

    r.run(function(results) {

      list.length.should.equal(numCallback)

      for(var i=0 ; i < numCallback ; i++) {

        list[i].should.equal(i)

      }

      var sum = 0;

      for(var j = 0 ; j < numCallback ; j++) {

        sum += results[j][0]

      }

      sum.should.equal(9+8+7+6+5+4+3+2+1)

      done();
    });

  });

  it(", not queuing any function should be valid", function(done) {

    var r = new ParallelRunner();

    r.run(function() {

      done();

    });

  });
});
