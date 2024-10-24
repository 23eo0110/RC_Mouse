package ga.gi.choju.micromousecontroller

import android.content.pm.ActivityInfo
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.view.MotionEvent
import android.view.View
import android.view.inputmethod.EditorInfo
import android.widget.EditText
import android.widget.LinearLayout
import android.widget.SeekBar
import android.widget.Space
import android.widget.TextView
import androidx.activity.enableEdgeToEdge
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import androidx.core.view.setPadding
import ga.gi.choju.micromousecontroller.databinding.ActivityMainBinding
import java.net.DatagramPacket
import java.net.DatagramSocket
import java.net.InetAddress
import java.util.Date

class MainActivity : AppCompatActivity() {
    private var screenWidth: Int = -1
    private var screenHeight: Int = -1

    private var steering = 0.0
    private var speed = 0.0

    private var turnRate = 0.0

    private var adjustRate = 0.0

    private var rightSpeed: Double = 0.0
    private var leftSpeed: Double = 0.0
    private var leftIndex: Int = -1
    private var rightIndex: Int = -1

    private var isRightXFixed = false
    private var isLeftYFixed = false

    private var rightStartY = -1f
    private var leftStartX = -1f

    private var singleTapCount = 0

    private var startEpocTime:Long = -1

    private lateinit var bind: ActivityMainBinding
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        bind = ActivityMainBinding.inflate(layoutInflater)
        setContentView(bind.root)
        ViewCompat.setOnApplyWindowInsetsListener(bind.main) { v, insets ->
            val systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars())
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom)
            insets
        }
        requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE

        val handler = Handler(Looper.getMainLooper())
        val sendStop = object : Runnable {
            override fun run() {
                if(leftIndex == -1 && rightIndex == -1) {
                    sendUdpMessage("{\"left_speed\": 0.0, \"right_speed\": 0.0}", "192.168.4.1", 8888)
                }
                handler.postDelayed(this, 1000)
            }
        }

        handler.postDelayed(sendStop, 1000)
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if(hasFocus){
            screenWidth = bind.main.width
            screenHeight = bind.main.height
        }
    }

    override fun onGenericMotionEvent(event: MotionEvent?): Boolean {
        if(event != null && event.action == MotionEvent.ACTION_MOVE){
//            Log.i(TAG, "onGenericMotionEvent: ${event.action}")
            bind.textView.text = getText(R.string.info_label)
            bind.textView.text = bind.textView.text.toString() + "action: ${event.action}\n"
            bind.textView.text = bind.textView.text.toString() + "L_x: ${event.getAxisValue(
                MotionEvent.AXIS_X)}, L_y: ${event.getAxisValue(MotionEvent.AXIS_Y)}\n"
            bind.textView.text = bind.textView.text.toString() + "R_x: ${event.getAxisValue(
                MotionEvent.AXIS_Z)}, R_y: ${event.getAxisValue(MotionEvent.AXIS_RZ)}\n"
            bind.textView.text = bind.textView.text.toString() + "L_trigger: ${event.getAxisValue(
                MotionEvent.AXIS_LTRIGGER)}\n"
            bind.textView.text = bind.textView.text.toString() + "R_trigger: ${event.getAxisValue(
                MotionEvent.AXIS_RTRIGGER)}\n"

            if(event.getAxisValue(MotionEvent.AXIS_LTRIGGER) > 0.2 && event.getAxisValue(MotionEvent.AXIS_RTRIGGER) > 0.2){
                speed = 0.0
            } else if(event.getAxisValue(MotionEvent.AXIS_LTRIGGER) > 0.2){
                val value = -(event.getAxisValue(MotionEvent.AXIS_LTRIGGER) - 0.2) * 1.25
                speed = value
            } else if(event.getAxisValue(MotionEvent.AXIS_RTRIGGER) > 0.2){
                val value = (event.getAxisValue(MotionEvent.AXIS_RTRIGGER) - 0.2) * 1.25
                speed = value
            } else {
                speed = 0.0
            }

            if(event.getAxisValue(MotionEvent.AXIS_Z) > 0.2){
                val value = (event.getAxisValue(MotionEvent.AXIS_Z) - 0.2) * 1.25
                steering = -value
            } else if(event.getAxisValue(MotionEvent.AXIS_Z) < -0.2){
                val value = (event.getAxisValue(MotionEvent.AXIS_Z) + 0.2) * 1.25
                steering = -value
            } else {
                steering = 0.0
            }

            bind.textView.text = bind.textView.text.toString() + "speed: $speed\nsteering: $steering\n"
            calcSpeed()

        }
        return super.onGenericMotionEvent(event)
    }

    override fun onTouchEvent(event: MotionEvent?): Boolean {
//        Log.i(TAG, "onTouchEvent: $event")
        if(event != null){
            bind.textView.text = "------------------------------ info -------------------------------\n"
            var isLeft = false
            var isRight = false

            for(num in 0..<event.pointerCount){
                val line = "pointerIndex: $num, x: ${event.getX(num)}, y: ${event.getY(num)}"
                if(!isLeft && event.getX(num) < screenWidth / 2){
                    isLeft = true
                    leftIndex = num
                }
                if(!isRight && event.getX(num) > screenWidth / 2){
                    isRight = true
                    rightIndex = num
                }
                bind.textView.text = bind.textView.text.toString() + line + "\n"
            }
            if(event.action == MotionEvent.ACTION_UP){
                bind.textView.text = "------------------------------ info -------------------------------\nRelease\n"
                leftIndex = -1
                rightIndex = -1
            }

            if(!isLeft){
                leftIndex = -1
            }
            if(!isRight){
                rightIndex = -1
            }

            if(leftIndex != -1){
//                bind.leftLabel.x = event.getX(leftIndex)
                if(!isLeftYFixed) {
                    bind.leftLabel.y = event.getY(leftIndex)
                    leftStartX = event.getX(leftIndex)
                    isLeftYFixed = true
                }
                var xSub = leftStartX - event.getX(leftIndex)
                if(xSub > (screenWidth / 8)){
                    xSub = screenWidth.toFloat() / 8
                } else if (xSub < -(screenWidth / 8)){
                    xSub = -(screenWidth.toFloat() / 8)
                }
                bind.leftLabel.visibility = View.VISIBLE
                bind.leftLabel.x = leftStartX - xSub
                steering = xSub / (screenWidth.toDouble() / 8)
            } else {
                bind.leftLabel.visibility = View.INVISIBLE
                isLeftYFixed = false
                leftStartX = -1f
                steering = 0.0
            }
            if(rightIndex != -1){
                if(!isRightXFixed){
                    bind.rightLabel.x = event.getX(rightIndex)
                    rightStartY = event.getY(rightIndex)
                    isRightXFixed = true
                }
                var ySub = rightStartY - event.getY(rightIndex)
                if(ySub > (screenHeight / 5)){
                    ySub = screenHeight.toFloat() / 5
                } else if (ySub < -(screenHeight / 5)){
                    ySub = -(screenHeight.toFloat() / 5)
                }
                bind.rightLabel.y = rightStartY - ySub
                bind.rightLabel.visibility = View.VISIBLE
                speed = ySub / (screenHeight.toDouble() / 5)
            } else {
                isRightXFixed = false
                rightStartY = -1f
                speed = 0.0
                bind.rightLabel.visibility = View.INVISIBLE
            }

            bind.textView.text = bind.textView.text.toString() + "leftIndex: $leftIndex, rightIndex: $rightIndex\nspeed: $speed\nsteering: $steering\n"

            if(event.pointerCount > 1){
                singleTapCount = 0
            }
            if(event.action == MotionEvent.ACTION_UP){
                singleTapCount++
                if(singleTapCount == 1){
                    startEpocTime = Date().time
                } else {
                    if(Date().time - startEpocTime > 1000){
                        singleTapCount = 0
                    }
                }
                if(singleTapCount > 4){
                    singleTapCount = 0
                    AlertDialog.Builder(this)
                        .setView(LinearLayout(this).apply {
                            orientation = LinearLayout.VERTICAL
                            setPadding(12)
                            val turnRateText = TextView(this@MainActivity).apply {
                                text = getTurnRateText()
                            }
                            addView(turnRateText)
                            addView(SeekBar(this@MainActivity).apply {
                                max = 20
                                progress = (turnRate * 10).toInt() + 10
                                setOnSeekBarChangeListener(object: SeekBar.OnSeekBarChangeListener{
                                    override fun onProgressChanged(
                                        seekBar: SeekBar?,
                                        progress: Int,
                                        fromUser: Boolean
                                    ) {
                                        val value = progress - 10
                                        turnRate = value.toDouble() / 10
                                        turnRateText.text = getTurnRateText()
                                    }

                                    override fun onStartTrackingTouch(seekBar: SeekBar?) {
                                        //
                                    }

                                    override fun onStopTrackingTouch(seekBar: SeekBar?) {
                                        //
                                    }

                                })
                            })
                            val adjustTextView = TextView(this@MainActivity).apply {
                                text = "左右差: ${String.format("%.2f", adjustRate)}"
                            }
                            val adjustSeekBar = SeekBar(this@MainActivity).apply {
                                max = 2000
                                progress = (adjustRate * 1000).toInt() + 1000
                                setOnSeekBarChangeListener(object: SeekBar.OnSeekBarChangeListener{
                                    override fun onProgressChanged(
                                        seekBar: SeekBar?,
                                        progress: Int,
                                        fromUser: Boolean
                                    ) {
                                        val value = progress - 1000
                                        adjustRate = value.toDouble() / 1000
                                        adjustTextView.text = "左右差: ${String.format("%.2f", adjustRate)}"
                                    }

                                    override fun onStartTrackingTouch(seekBar: SeekBar?) {
                                        //
                                    }

                                    override fun onStopTrackingTouch(seekBar: SeekBar?) {
                                        //
                                    }

                                })
                            }
                            addView(adjustTextView)
                            addView(adjustSeekBar)
                        })
                        .setTitle("回転率変更")
                        .setPositiveButton("OK", null)
                        .show()

                }
            }

        }
        calcSpeed()
        return super.onTouchEvent(event)
    }

    fun calcSpeed(){
        // steering 左1 右-1
        if(steering > 0){
            leftSpeed = speed + (speed * (turnRate - 1) * steering)
            rightSpeed = speed
        } else if(steering < 0) {
            rightSpeed = speed + (speed * (turnRate - 1) * -steering)
            leftSpeed = speed
        } else {
            rightSpeed = speed
            leftSpeed = speed
        }

        if(adjustRate > 0){
            leftSpeed = (leftSpeed - leftSpeed * adjustRate * 0.1)
        } else if(adjustRate < 0){
            rightSpeed = (rightSpeed + rightSpeed * adjustRate * 0.1)
        }

        bind.textView.text = bind.textView.text.toString() + "leftSpeed: $leftSpeed\nrightSpeed: $rightSpeed"
//        Log.i(TAG, "calcSpeed: {\"left_speed\": $leftSpeed, \"right_speed\": $rightSpeed}")
        sendUdpMessage("{\"left_speed\": $leftSpeed, \"right_speed\": $rightSpeed}", "192.168.4.1", 8888)

    }

    fun getTurnRateText(): String{
        val text = if(turnRate > 0){
            "緩旋回"
        } else if(turnRate < 0){
            "超信地旋回"
        } else {
            "信地旋回"
        }
        return "回転率: ${(turnRate * 100).toInt()}%, $text"
    }

    private fun sendUdpMessage(message: String, address: String, port: Int) {
        Thread{

            var isSuccess = false
            var sendCount = 0
            while(!isSuccess && sendCount < 10) {
                try {
                    val socket = DatagramSocket()
                    val buffer = message.toByteArray()
                    val inetAddress = InetAddress.getByName(address)
                    val packet = DatagramPacket(buffer, buffer.size, inetAddress, port)
                    socket.send(packet)
                    socket.close()
                    isSuccess = true
                } catch (e: Exception) {
                    Log.w(TAG, "sendUdpMessage: 送信失敗", e)
                    sendCount++
                }
            }
        }.start()
    }

    companion object{
        private const val TAG = "MainActivity"
    }
}