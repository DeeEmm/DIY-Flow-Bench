'use strict';
var assign = require('object-assign');
var chalk = require('chalk');
var gutil = require('gulp-util');
var through = require('through2');
var removeHtmlComments = require('remove-html-comments');

module.exports = function(opts) {
  opts = assign({
    // TODO: remove this when gulp get's a real logger with levels
    verbose: process.argv.indexOf('--verbose') !== -1
  }, opts);

  return through.obj(function(file, enc, cb) {
    if (file.isNull()) {
      cb(null, file);
      return;
    }

    if (file.isStream()) {
      cb(new gutil.PluginError('gulp-remove-html-comments', 'Streaming not supported'));
      return;
    }

    var result = removeHtmlComments(file.contents.toString());
    file.contents = new Buffer(result.data);

    var comments = result.comments;
    var removedCount = comments.length;

    if (opts.verbose && removedCount > 0) {
      comments.forEach(function(comment) {
        gutil.log(chalk.red(comment) + chalk.magenta('removed'));
      });

      gutil.log(file.path + ': ' + chalk.red('✖ ') + removedCount + ' comments removed.');
    }

    cb(null, file);
  });
};
