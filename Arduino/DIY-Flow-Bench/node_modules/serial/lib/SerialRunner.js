/** Very simple chaining mecanism to serially execute asynchronous functions
 *
 */

function SerialRunner() {

  this.last
  this.first

  /** first arg if the function to call.
   * Other args are params.
   */
  this.add = function() {

    var func = arguments[0]

    var args = []
    for(var i=1; i < arguments.length ; i++) {
      args.push(arguments[i])
    }


    if(!this.first) {
      this.first = new ChainItem(func, args)
      this.last = this.first
    } else {
      var chainItem = new ChainItem(func, args)
      this.last.next = chainItem
      this.last = chainItem
    }

    return this
  }

  this.run = function(done) {

    if(stopped) {
      throw new Error("The process has been stopped and cannot be recovered.")
    }

    if(done) {
      this.add(done)
    }

    if(this.first) {
      this.first.run()
    } else {
      throw new Error("You must call add() before calling run()")
    }
  }

  var errorHandler;

  this.onError = function(func) {
    errorHandler = func
  };

  function handleError(err) {
    if(errorHandler) {
      errorHandler(err)
    }
  }

  var stopped = false

  this.stop = function() {
    stopped = true
  }

  function ChainItem(func, args) {
    this.args = args;
    this.func = func;
    var self = this;

    function handleNext(err) {

      if(err) {
        handleError(err)
      }

      if(self.next) {
        self.next.run();
      } else {
        console.log("ERR: SerialRunner --> no next");
      }
    }

    this.run = function() {
      var self = this;
      setImmediate(function() {
        if(stopped) {
          return;
        }
        try {
          if(self.args) {
            self.args.push(handleNext);

            func.apply(self, self.args);
          } else {
            func(handleNext);
          }
        } catch(err) {
          handleNext(err);
        }
      })
    }
  }
}

module.exports = SerialRunner;
