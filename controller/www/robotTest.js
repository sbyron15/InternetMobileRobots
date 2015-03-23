$(document).ready(function() {

    var REQUEST_PATH = '/arduino/status';

    function processResults(results){
        var allResults = '';
        var sum = 0;

        for (var i = 0; i < results.length; i++){
            allResults += results[i] + ', ';
            sum += results[i];           
        }
        $('#latency-mean').text(sum * 1.0 / results.length);
        $('#latency-median').text(results[results.length / 2]);
        $('#latency-raw').text(allResults);

        enableInputs();
        $('#latency-run').text('Run latency test');
    }

    function sendSequentialRequests(numRequests, results){
        var start = Date.now();
        $.get(REQUEST_PATH, function(data){
            results[numRequests - 1] = Date.now() - start;
            
            if (numRequests == 1) {
                processResults(results);
            }
            else {
                sendSequentialRequests(numRequests - 1, results);
            }
        });
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
});
