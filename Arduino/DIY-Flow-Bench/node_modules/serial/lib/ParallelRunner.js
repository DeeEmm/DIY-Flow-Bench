
function ParallelRunner() {

  var self = this

  var chainItems = []

  var stopOnError = false

  this.add = function() {

    var func = arguments[0]

    var args = []

    for(var i=1; i < arguments.length ; i++) {

      args.push(arguments[i])

    }

    chainItems.push(new ChainItem(func, args))

    return this
  }

  this.run = function(done) {

    var counter = chainItems.length

    var results = []

    function makeCallback (index) {

      return function () {

        counter --

        var result = []

        // we use the arguments object here because some callbacks
        // in Node pass in multiple arguments as result.
        for (var i = 0 ; i < arguments.length ; i++) {
          result.push(arguments[i])
        }

        results.push(result)

        if (counter === 0) {

          if(done) {
            done(results)
          }
        }
      }

    }

    if(counter > 0) {
      for (var i = 0 ; i < chainItems.length ; i++) {

        chainItems[i].run(makeCallback(i))

      }
    } else if(done) {
      process.nextTick(done);
    }
  }

  this.setStopOnError = function(value) {

    stopOnError = value

  }

  function ChainItem(func, args) {

    this.args = args

    this.func = func

    var self = this

    this.run = function(callback) {

      if(self.args) {

        self.args.push(callback)
        func.apply(this, self.args)

      } else {

        func(callback)

      }
    }
  }
}

module.exports = ParallelRunner
