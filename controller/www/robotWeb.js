$(document).ready(function() {
    // Debug
    var DEBUG = true;
    if (!DEBUG) $('#debug').hide();

    // Constants
    var UP = 'up';
    var DOWN = 'down';
    var RIGHT = 'right';
    var LEFT = 'left';
    var STOP = 'stop';
    var MIN_SPEED = 0;
    var MAX_SPEED = 3;
    var UPDATE_DELAY = 5000;

    // active direction
    var direction = STOP;

    // web command states
    var sending = false;
    var updating = false;

    var sid = null;

    $('#error').hide();

    // ensure no speed, browsers often remember previous setting
    $('#speedControl').val(MIN_SPEED);

    // periodically update status (AI can override control)
    setInterval(function(){
        updateStatus();
    }, UPDATE_DELAY);

    // Key events
    $(document).on('keydown', function(e) { 
        $('#msg').text(e.keyCode + ' key pressed'); 
        
        var sc = $('#speedControl');
        if (isSpeedUp(e.keyCode) && parseInt(sc.val()) < MAX_SPEED){
            sc.val(parseInt(sc.val()) + 1);
        }
        else if (isSlowDown(e.keyCode) && parseInt(sc.val()) > MIN_SPEED){
            sc.val(parseInt(sc.val()) - 1);
        }
        else if (isLeft(e.keyCode)) sendDirection(LEFT);
        else if (isUp(e.keyCode)) sendDirection(UP);
        else if (isRight(e.keyCode)) sendDirection(RIGHT);
        else if (isDown(e.keyCode)) sendDirection(DOWN);
    }); 
    $(document).on('keyup', function(e) { 
        $('#msg').text(e.keyCode + ' key released');
        
        if (isDirectional(e.keyCode)) {
            sendDirection(STOP);
        }
    });

    // Direction click event
    $('.direction-box').on('click', function(){
        var id = $(this).children('img').attr('id');

        if (id == direction) sendDirection(STOP);
        else sendDirection(id);
    });

    // 
    $('#reconnect').on('click', function(){
        $(this).attr('disabled', 'disabled');
        $(this).text('Connecting...');

        getSID(function(){
            $('#reconnect').text('Connected!');
            setTimeout(function(){ $('#error').hide(); }, 1000);
        });
    });

    // debug control buttons
    $('#clear-log').on('click', function(){
        $('#msg').load('/arduino/clearLog');
    });
    $('#force-update').on('click', function(){
        updateStatus();
    });
    
    // Keyboard helper functions
    function isLeft(keyCode){
        return keyCode == 37 || keyCode == 65;
    }
    function isRight(keyCode){
        return keyCode == 39 || keyCode == 68;
    }
    function isUp(keyCode){
        return keyCode == 38 || keyCode == 87;
    }
    function isDown(keyCode){
        return keyCode == 40 || keyCode == 83;
    }
    function isDirectional(keyCode){
        return isLeft(keyCode) || isRight(keyCode) || isUp(keyCode) || isDown(keyCode);
    }
    function isSpeedUp(keyCode){
        return keyCode == 67 || keyCode == 190;
    }
    function isSlowDown(keyCode){
        return keyCode == 88 || keyCode == 188;
    }

    // Speed change event
    $('#speedControl').on('change', function(){
        $('#msg').text('Setting speed to level ' + $(this).val()); 
        switch (parseInt($(this).val())){
            case 0: 
                $('#speed').load('/arduino/stop');
                break;
            case 1:
                $('#speed').load('/arduino/setSlowSpeed');
                break;
            case 2:
                $('#speed').load('/arduino/setMediumSpeed');
                break;
            case 3:
                $('#speed').load('/arduino/setFastSpeed');
                break;
            default:
                $('#speed').text('ERROR'); 
        }
    });

    // Sends direction to the server, page updated on response
    function sendDirection(id){
        // Block consecutive call if response is pending. 
        // Always allow stop command, unless stop is the pending response
        if (sending && (id != STOP || direction == STOP)) return; 

        sending = true;

        var path = '';
        var debugMsg = '';

        if (id == UP){
            path = '/arduino/moveForward';
            debugMsg = 'Setting direction to forward';
        }
        else if (id == DOWN){
            path = '/arduino/moveBackward';
            debugMsg = 'Setting direction to reverse';
        }
        else if (id == LEFT){
            path = '/arduino/leftTurn';
            debugMsg = 'Setting direction to left';
        }
        else if (id == RIGHT){
            path = '/arduino/rightTurn';
            debugMsg = 'Setting direction to right';
        }
        else { // id is stop, or unrecognized, stop is failsafe.
            id = STOP;
            path = '/arduino/stop';
            debugMsg = 'Stopping';
        }

        $('#direction').load(path, function(){ 
            // on response, update page and state
            setDirection(id);
            sending = false;
        });

        $('#msg').text(debugMsg);
    }

    function updateStatus(){
        // don't bother updating if change will happen anyways
        if (sending || updating) return;
        updating = true;

        $('#status').text('Requesting status');

        $('#status').load('/arduino/status', function(){
            var status = $('#status').text();
            var index = status.indexOf(':');

            if (index >= 0){
                var id = STOP;
                var robotDirection = status.slice(0, index).trim();
                var robotSpeed = status.slice(index + 1).trim();          

                if (robotDirection == 'moveForward') id = UP;
                else if (robotDirection == 'moveBackward') id = DOWN;
                else if (robotDirection == 'leftTurn') id = LEFT;
                else if (robotDirection == 'rightTurn') id = RIGHT;

                setDirection(id);
                
                $('#direction').text(id);
                $('#speed').text(robotSpeed);
            }
            else $('#status').text('Status update failed');

            updating = false;
        });
    }

    // set UI direction to reflect robot direction
    function setDirection(id){
        $('#' + direction).attr('src', 'img/' + direction + '_off.png');
        $('#' + id).attr('src', 'img/' + id + '_on.png');
        direction = id;
    }

    // handles sending commands, including the session id
    function sendCommand(htmlId, commandPath){
        if (sid == null){
            getSID(function(){
                sendCommand(htmlId, commandPath);
            });
        }
        else { 
            // we have a session id, execute command
            $('#' + htmlId).load(commandPath + '/' + sid, function(){
                if ($('#' + htmlId).text().slice(5) == 'err=0'){ // session id expired
                    sid = null;
                    getSID(function(){
                        sendCommand(htmlId, commandPath);
                    });
                }
                else if ($('#' + htmlId).text().slice(5) == 'err=2'){ // sid expired and a new sid was issued
                    $('#error').show();
                }
                else { //successful command
                    $('#error').hide();
                }
            });
        }
    }

    function getSID(onSuccess){
        getSID(onSuccess, function(){
            setError();
        });
    }
    function getSID(onSuccess, onFailure){
        // attempt to get SID
        $('#sid').load('/arduino/getSID', function(){
            if ($('#sid').text().slice(5) == 'err=1'){ // another session is in progress
                sid = null;
                onFailure();
            }
            else { // sid received from robot
                sid = $('#sid').text();
                onSuccess();
            }
        });
    }
    function setError(){
        $('#error').show();
        $('#error-msg').text('Command failed: another user may be controlling the robot.');
        $('#reconnect').removeAttr('disabled');
        $('#reconnect').text('Connection failed: try again');
    }
});
