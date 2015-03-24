$(document).ready(function() {

    var REQUEST_PATH = '/arduino/status';
    var PRECISION = 3;

    function processLatencyResults(results){
        var allResults = '';
        var sum = 0, squareSum = 0, mean = 0, variance = 0, sd = 0, error = 0;

        for (var i = 0; i < results.length; i++){
            allResults += results[i] + ', ';
            sum += results[i]; 
            squareSum += results[i] * results[i];        
        }
        mean = sum * 1.0 / results.length;
        variance = (squareSum + (sum * sum * 1.0 / results.length)) / (results.length - 1);
        sd = Math.sqrt(variance);
        error = 1.96 * sd / Math.sqrt(results.length);

        $('#latency-mean').text(mean.toFixed(PRECISION) + ' ± ' + error.toFixed(PRECISION) + ' with 95% confidence');
        $('#latency-median').text(results[results.length / 2].toFixed(PRECISION));
        $('#latency-sd').text(sd.toFixed(PRECISION));
        $('#latency-raw').text(allResults);

        enableInputs();
        $('#latency-run').text('Run latency test');
    }

    function processThroughputResults(responses, lastResponseTime){       
        $('#tp-time').text((lastResponseTime * 1.0 / 1000).toFixed(PRECISION));
        $('#tp-responses').text(responses);
        $('#tp-tp').text((responses * 1.0 / lastResponseTime).toFixed(PRECISION));

        enableInputs();
        $('#tp-run').text('Run throughput test');
    }

    function sendSequentialRequests(numRequests, results){
        var start = Date.now();
        $.get(REQUEST_PATH, function(data){
            results[numRequests - 1] = Date.now() - start;
            
            if (numRequests == 1) {
                processLatencyResults(results);
            }
            else {
                sendSequentialRequests(numRequests - 1, results);
            }
        });
    }

    function sendAsynchronousRequests(numRequests, timeoutInMS){
        var responses;
        var lastResponseTime;
        var start = Date.now();

        var timer = setTimeout(function(){
            processThroughputResults(responses, lastResponseTime);
        }, timeoutInMS);

        for (var i = 0; i < numRequests; i++){
            $.get(REQUEST_PATH, function(data){
                var now = Date.now();

                if ((now - start) < timeoutInMS){
                    responses++;
                    lastResponseTime = now;

                    if (responses == numRequests){
                        clearTimeout(timer);
                        processThroughputResults(responses, lastResponseTime);
                    }
                }
            });
        }
    }

    function enableInputs(){
        $('input').removeAttr('disabled');
        $('button').removeAttr('disabled');
    }

    function disableInputs(){
        $('input').attr('disabled', 'disabled');
        $('button').attr('disabled', 'disabled');
    }

    $('#latency-run').on('click', function(){
        disableInputs();
        $('#latency-run').text('Running latency test...');

        var numRequests = parseInt($('#latency-number').val(), 10);
        var results = new Array(numRequests);
        sendSequentialRequests(numRequests, results);
    });

    $('#tp-run').on('click', function(){
        disableInputs();
        $('#tp-run').text('Running throughput test...');

        sendAsynchronousRequests($('#tp-number').val(), $('#tp-timeout').val() * 6000);
    });
});
