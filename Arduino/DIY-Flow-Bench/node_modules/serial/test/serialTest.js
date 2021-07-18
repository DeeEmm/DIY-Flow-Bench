var SerialRunner = require("../index.js").SerialRunner

var should = require("should")

describe("When running tasks sequentially", function() {
  it("", function(done) {
    var r = new SerialRunner()
    var list = []
    var numCallback = 10
    var func = function(i, next) {
      setTimeout(function() {
        list.push(i)
        next()
      }, 20-i)
    }

    for(var i = 0 ; i < numCallback ; i++) {
      r.add(func, i)
    }

    r.run(function() {
      list.length.should.equal(numCallback)
      for(var i=0 ; i < numCallback ; i++) {
        list[i].should.equal(i)
      }
      done()
    })

  })

  it(", not queuing any function should be valid", function(done) {

    var r = new SerialRunner()

    r.run(function() {

      done()

    })

  })

  it(", an error should bubble to the error handler", function(done) {

    var r = new SerialRunner()

    r.add(function(callback) {
      callback(new Error("this error should rise to the error handler"))
    })

    r.onError(function(err) {
      r.stop()
      setTimeout(done, 1000)
    })

    r.run(function() {

      should.fail("The end callback should not be called when an error is catched")

    })

  })

  it(", an error should bubble to the error handler but the runner should keep going if stop() is not called", function(done) {

    var r = new SerialRunner()

    r.add(function(callback) {
      callback(new Error("this error should rise to the error handler"))
    })

    var errorCalled = false

    r.onError(function(err) {
      errorCalled = true
    })

    r.run(function() {
      should.exist(errorCalled)
      done()
    })

  })

  it(", an unhandled error should bubble to the error handler", function(done) {

    var r = new SerialRunner()

    r.add(function(callback) {
      throw new Error("this error should rise to the error handler")
    })

    r.onError(function(err) {
      r.stop()
      setTimeout(done, 1000)
    })

    r.run(function() {

      should.fail("The end callback should not be called when an error is catched")

    })

  })
})
