#!/usr/bin/env node
'use strict';
var chalk = require('chalk');
var concat = require('concat-stream');
var fs = require('fs');
var meow = require('meow');
var removeHtmlComments = require('./');

var cli = meow({
  help: [
    'Usage',
    '  remove-html-comments <input-path>',
    '  remove-html-comments <input-path> <output-path>',
    '  cat <input-path> | remove-html-comments',
    '',
    'Example',
    '  remove-html-comments index.html',
    '  remove-html-comments index.html no-comments.html',
    '  cat index.html | remove-html-comments',
    '  cat index.html | remove-html-comments > no-comments.html',
    '',
    'Options',
    '  -v, --verbose    Verbose output'
  ].join('\n')
}, {
  alias: {
    v: 'verbose'
  }
});

var input = cli.input;
var inputPath = input[0];
var outputPath = input[1];
var verbose = cli.flags.verbose;

function extraInfo(comments) {
  if (!verbose) {
    return;
  }

  console.log('');

  var removedCount = comments.length;

  if (removedCount > 0) {
    comments.forEach(function(comment) {
      console.log(chalk.red(comment) + chalk.magenta('removed'));
    });

    console.log(chalk.red('✖ ') + removedCount + ' comments removed.');
  }
}

if (process.stdin.isTTY) {
  if (!inputPath) {
    console.error('Expected an input path');
    process.exit(1);
  }

  fs.readFile(inputPath, function(err, data) {
    if (err) {
      console.error(err.toString());
      process.exit(1);
    }

    var result = removeHtmlComments(data);

    if (outputPath) {
      fs.writeFile(outputPath, result.data, function(err) {
        if (err) {
          console.error(err.toString());
          process.exit(1);
        }
      });
    } else {
      process.stdout.write(result.data);
    }

    extraInfo(result.comments);
  });
} else {
  process.stdin
    .pipe(concat(function(data) {
      var result = removeHtmlComments(data);
      process.stdout.write(result.data);
      extraInfo(result.comments);
    }));
}
