'use strict';

var nano        = require('cssnano'),
    assign      = require('object-assign'),
    PluginError = require('gulp-util').PluginError,
    Transform   = require('stream').Transform,
    applySourceMap = require('vinyl-sourcemaps-apply'),

    PLUGIN_NAME = 'gulp-cssnano';

module.exports = function(options) {
    options = options || {};
    var stream = new Transform({objectMode: true});

    stream._transform = function(file, encoding, cb) {
        if (file.isNull()) {
            return cb(null, file);
        }
        if (file.isStream()) {
            var error = 'Streaming not supported';
            return cb(new PluginError(PLUGIN_NAME, error));
        } else if (file.isBuffer()) {
            try {
                var result = nano.process(String(file.contents), assign(options, {
                    map: (file.sourceMap) ? {annotation: false} : false,
                    from: file.relative,
                    to: file.relative
                }));
                if (result.map && file.sourceMap) {
                    applySourceMap(file, String(result.map));
                    file.contents = new Buffer(result.css);
                } else {
                    file.contents = new Buffer(result);
                }

                this.push(file);
            } catch (e) {
                var p = new PluginError(PLUGIN_NAME, e, {fileName: file.path});
                this.emit('error', p);
            }
            cb();
        }
    };

    return stream;
};
