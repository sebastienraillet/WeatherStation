package com.example.myapplication3.app;

import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;

/**
 * Created by camillemuller on 14/05/2014.
 */
 public  class SerialTime {

    static String sendTimeMessage(String header, long time) {
        String timeStr = String.valueOf(time);

        return header + timeStr + '\n';
    }

   static  long getTimeNow(){
        // java time is in ms, we want secs
        Date d = new Date();
        Calendar cal = new GregorianCalendar();
        long current = d.getTime()/1000;
        long timezone = cal.get(cal.ZONE_OFFSET)/1000;
        long daylight = cal.get(cal.DST_OFFSET)/1000;
        return current + timezone + daylight;
    }
}
