Serial
====

Serial is a Node.JS module to serially/sequentially run asynchronous functions.

It also includes a Parallel runner to simultaneously run asynchronous functions.

Usage
====

SerialRunner
------------

```
  var SerialRunner = require("serial").SerialRunner;
  var runner = new SerialRunner();
  
  runner.add(function1, param1).add(function2, param2);
  runner.add(function3, param3);

  runner.onError(function(err) {
    runner.stop(); // stop further queued function from being run
    console.log("There was an error");
  });
  
  runner.run(function() {
    console.log("done");
  });


  function function1(param, callback) {
    // do smthg

    if("something went wrong") {
      var err = new Error("something went wrong");
      callback(err);
    } else {
      callback();
    }
  }
```

Parallel Runner
---------------

```
  var ParallelRunner = require("serial").ParallelRunner;
  var runner = new ParallelRunner();

  runner.add(function1, param1).add(function2, param2);
  runner.add(function3, param3);

  runner.run(function() {
    console.log("done");
  });


  function function1(param, callback) {
    // do smthg
    callback();
  }
```

MIT License
===========
Copyright (c) 2012 Model N, Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
