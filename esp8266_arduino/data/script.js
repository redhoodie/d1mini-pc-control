var esp_timed_out = false;

function process_unixtimestamp(unix_timestamp) {
  try {
    var a = new Date(unix_timestamp * 1000);
    var months = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];
    var year = a.getFullYear();
    var month = months[a.getMonth()];
    var date = a.getDate();
    var hour = a.getHours();
    var min = a.getMinutes();
    var sec = a.getSeconds();
    var time = date + ' ' + month + ' ' + year + ' ' + hour + ':' + min + ':' + sec;
    return time;
  } catch (err) {
    return unix_timestamp;
  }
}

function process_line(line) {
  if (line.trim() == "") {
    return "";
  }
  words = line.split(" ");
  unix_timestamp = words[0];
  if (!Number.isNaN(unix_timestamp)) {
    processed_date = process_unixtimestamp(unix_timestamp);
  }
  else {
    processed_date = unix_timestamp;
  }

  words[0] = processed_date;
  return words.join(" ");
}

(function ($) {
  var _loadThen = $.fn.loadThen;

  /**
   * An alternative to jQuery"s ajax load() function that has the same interface 
   * except it does not take a callback function, but instead it returns a promise.
   * The promise will be resolved when the content are loaded into the set of matched elements.
   * @param url {string} A string containing the URL to which the request is sent.
   * @param params {object or string} A plain object or string that is sent to the server with the request.
   * @returns {jQuery Promise} with params (html, status, jqXHR). 
   * When the promise is resolved, functions The [this] object is the set of matched elements.
   */
  $.fn.loadThen = function (url, params, flip = false) {
    if (typeof url !== "string" && _loadThen) {
      return _loadThen.apply(this, arguments);
    }

    if (this.length <= 0) {
      return jQuery.Deferred().resolveWith(this, [""]);
    }

    var selector, type, response,
      self = this,
      off = url.indexOf(" ");

    if (off >= 0) {
      selector = jQuery.trim(url.slice(off));
      url = url.slice(0, off);
    }

    if (params && typeof params === "object") {
      type = "POST";
    }

    return jQuery.ajax({
      url: url,
      timepout: 3000,
      type: type,
      dataType: "html",
      error: function (jqXHR, textStatus, errorThrown) {
        if (textStatus == "error") {
          esp_timed_out = true;
          if ($("#esp-status").hasClass("alert-success")) {
            $("#esp-status").addClass("alert-danger").removeClass("alert-success").text("ESP Timeout");
          }
        }
      },
      data: params
    }).then(function (responseText) {
      if (flip) {
        responseText = responseText.split("\n").reverse().map(process_line).join("\n");
      }
      self.html(selector ? jQuery("<div>").append(jQuery.parseHTML(responseText)).find(selector) : responseText);

      if (esp_timed_out) {
        location.reload();
      }
      return self;
    });
  };
}(jQuery));

$(function () {
  function refreshContent() {
    $("#state_control").parent().loadThen("/ #state_control", false, false);
    $("#log").loadThen("/log.txt", false, true);
  }

  var timeoutID = window.setInterval(refreshContent, 5000);

  $('body').on('click', '#state_control .btn', function (e) {
    $.ajax({
      url: $(this).attr('href')
    }).done(function () {
      refreshContent();
    });
  });
});
