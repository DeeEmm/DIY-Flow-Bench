'use strict';
var regex = require('html-comment-regex');

module.exports = function(data) {
  data = String(data);
  var comments = [];
  data = data.replace(regex, function(match) {
    comments.push(match);
    return '';
  });

  return {
    data: data,
    comments: comments
  };
};
