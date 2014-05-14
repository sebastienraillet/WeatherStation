package com.example.myapplication3.app;

import android.graphics.Color;
import android.graphics.Paint;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.ActionBarActivity;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.LinearLayout;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import org.achartengine.ChartFactory;
import org.achartengine.GraphicalView;
import org.achartengine.chart.BarChart;
import org.achartengine.chart.CubicLineChart;
import org.achartengine.chart.LineChart;
import org.achartengine.chart.PointStyle;
import org.achartengine.model.XYMultipleSeriesDataset;
import org.achartengine.model.XYSeries;
import org.achartengine.renderer.XYMultipleSeriesRenderer;
import org.achartengine.renderer.XYSeriesRenderer;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Timer;





public class MainActivity extends ActionBarActivity {



     TextView unT;
   volatile static BluetoothTest test;
    Thread thread;
    Handler m_handler;
    Runnable m_handlerTask ;
    int timeleft=10;

    Timer t;
    String un;

    private GraphicalView mChart;

    private XYMultipleSeriesDataset mDataset = new XYMultipleSeriesDataset();

    private XYMultipleSeriesRenderer mRenderer = new XYMultipleSeriesRenderer(2);

    private XYSeries mTemperatureSeries;
    private XYSeries mHumidite;
    private XYSeries mPression;


    private XYSeriesRenderer mTempRenderer;
    private XYSeriesRenderer mHumidity;


    private void initChart() {
        mTemperatureSeries = new XYSeries("Température de la journée en cours") {
        };

        mHumidite = new XYSeries("Humidité de la journée en cours") {
        };

        mHumidity = new XYSeriesRenderer();
        mHumidity.setColor(Color.YELLOW);
        mHumidity.setPointStyle(PointStyle.CIRCLE);
        mHumidity.setFillPoints(true);
        mHumidity.setLineWidth(2);
        mHumidity.setDisplayChartValues(true);



        mDataset.addSeries(mTemperatureSeries);
        mDataset.addSeries(mHumidite);


        mTempRenderer = new XYSeriesRenderer();
        mHumidity = new XYSeriesRenderer();

        mTempRenderer.setColor(Color.RED);


        mRenderer.addSeriesRenderer(mTempRenderer);
        mRenderer.addSeriesRenderer(mHumidity);

        int length = mRenderer.getSeriesRendererCount();
        for (int i = 0; i < length; i++) {
            XYSeriesRenderer r = (XYSeriesRenderer) mRenderer.getSeriesRendererAt(i);
                r.setLineWidth(3f);
            }


        mRenderer.addYTextLabel(50, "50%",0);
        mRenderer.addYTextLabel(70, "60%",1);
        mRenderer.setYLabelsColor(0,Color.YELLOW);
     mRenderer.setYLabelsColor(1,Color.YELLOW);


        mRenderer.setYAxisAlign(Paint.Align.RIGHT, 1);
        mRenderer.setYLabelsAlign(Paint.Align.LEFT, 1);


        mRenderer.setYLabels(0);
        mRenderer.setXLabels(0);

        mRenderer.setBarSpacing(4);

        mRenderer.addXTextLabel(1,"00:00");
        mRenderer.addXTextLabel(2,"01:00");
        mRenderer.addXTextLabel(3,"02:00");
        mRenderer.addXTextLabel(4,"03:00");
        mRenderer.addXTextLabel(5,"04:00");
        mRenderer.addXTextLabel(6,"05:00");
        mRenderer.addXTextLabel(7,"06:00");
        mRenderer.addXTextLabel(8,"07:00");
        mRenderer.addXTextLabel(9,"08:00");
        mRenderer.addXTextLabel(10,"09:00");
        mRenderer.addXTextLabel(11,"10:00");
        mRenderer.addXTextLabel(12,"11:00");
        mRenderer.addXTextLabel(13,"12:00");
        mRenderer.addXTextLabel(14,"13:00");
        mRenderer.addXTextLabel(15,"14:00");
        mRenderer.addXTextLabel(16,"15:00");
        mRenderer.addXTextLabel(17,"16:00");
        mRenderer.addXTextLabel(18,"17:00");
        mRenderer.addXTextLabel(19,"18:00");
        mRenderer.addXTextLabel(20,"19:00");
        mRenderer.addXTextLabel(21,"20:00");
        mRenderer.addXTextLabel(22,"21:00");
        mRenderer.addXTextLabel(23,"22:00");
        mRenderer.addXTextLabel(24,"23:00");



    }

    private void addSampleData() {

        mTemperatureSeries.add(5, 30);
        mTemperatureSeries.add(6, 25);
        mTemperatureSeries.add(7, 35);
        mTemperatureSeries.add(8, 24);
        mTemperatureSeries.add(9, 14);
        mTemperatureSeries.add(10, 20);
        mTemperatureSeries.add(11, 13);
        mTemperatureSeries.add(11, 20);



        mHumidite.add(5, 50);
        mHumidite.add(6, 41);
        mHumidite.add(7, 45);
        mHumidite.add(8, 44);
        mHumidite.add(9, 55);
        mHumidite.add(10, 55);
        mHumidite.add(11, 46);
        mHumidite.add(12, 66);





    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);







        setContentView(R.layout.activity_main);

        Button refresh  = (Button) findViewById(R.id.bpSend);

        Switch connection = (Switch) findViewById(R.id.Connection);




        LinearLayout layout = (LinearLayout) findViewById(R.id.charts);
        if (mChart == null) {
            initChart();
            addSampleData();
           // mChart = ChartFactory.getCubeLineChartView(this, mDataset, mRenderer, 0.3f);

            String[] types = new String[] {LineChart.TYPE , BarChart.TYPE };

            // Creating a combined chart with the chart types specified in types array
            mChart = (GraphicalView) ChartFactory.getCombinedXYChartView(getBaseContext(), mDataset, mRenderer, types);


            layout.addView(mChart);

        } else {
            mChart.repaint();
        }

        unT =   (TextView) findViewById(R.id.textDEBOG);

        test = new BluetoothTest(getApplicationContext(),unT);

        t = new Timer();



        connection.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                                                  @Override
                                                  public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                                                      if(b)
                                                      {
                                                          try
                                                          {
                                                              test.findBT();
                                                              test.openBT();



                                                          } catch (IOException er)
                                                          {

                                                          }
                                                      }
                                                      else
                                                      {
                                                          try {
                                                                test.closeBT();
//                    thread.stop();


                                                          }catch (Exception e)
                                                          {
                                                              Toast.makeText(getApplicationContext(), "L'apparail n'est pas déconnecter ", Toast.LENGTH_LONG).show();

                                                          }
                                                      }
                                                  }
                                              });



                refresh.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View view) {

                        try {
                            java.util.Date date = new java.util.Date();

                            test.sendData("T" + System.currentTimeMillis() / 1000);
                            test.sendData("A");
                        } catch (IOException e) {
                            //TROLOLOLO
                        }
                    }
                });







        unT.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence charSequence, int i, int i2, int i3) {

            }

            @Override
            public void onTextChanged(CharSequence charSequence, int i, int i2, int i3) {

            }

            @Override
            public void afterTextChanged(Editable editable) {
            traitementchaine();
            }
        });


    }






    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();
        if (id == R.id.action_settings) {
            return true;
        }
        return super.onOptionsItemSelected(item);
    }


    void traitementchaine() {


        char T= 0xFF;

      //  String traitement[] = unT.getText().toString().split(";");






        if(unT.getText() != null)
        {
            if(unT.getText().toString().contains(Character.toString(T) ))
            {
                try
                {
                    System.out.println(SerialTime.sendTimeMessage(Character.toString(T), SerialTime.getTimeNow()));
                    test.sendData(SerialTime.sendTimeMessage(Character.toString(T), SerialTime.getTimeNow()));


                    // On supprime le caractére
                    int i =   unT.getText().toString().charAt(unT.getText().toString().indexOf(T));
                    if(unT.getText().length()>1)
                    unT.setText(unT.getText().subSequence(0,i-1).toString()+ unT.getText().subSequence(i+1,unT.getText().toString().length()).toString() );
                    else
                    return;

                } catch (IOException e)
                {

                }

            }



        String traitement[] = unT.getText().toString().split(";");

            if(traitement.length == 5)
      {
        for(int i=0; i<traitement.length;i++)
        {
            System.out.println(i);

            switch (i)
            {
                case 0:
                {
                    TextView unT = (TextView)findViewById(R.id.textAltitude);
                    unT.setText(traitement[i]);
                    break;
                }
                case 1:
                {
                    TextView unT = (TextView)findViewById(R.id.textTemp);
                    unT.setText(traitement[i]);
                    break;
                }
                case 2:
                {
                    TextView unT = (TextView)findViewById(R.id.textPression);
                    unT.setText(traitement[i]);
                    break;
                }
                case 3:
                {
                    TextView unT = (TextView)findViewById(R.id.textLum);
                    unT.setText(traitement[i]);
                    break;
                }
                case 4:
                {
                    TextView unT = (TextView)findViewById(R.id.textHum);
                    unT.setText(traitement[i]);
                    break;
                }

            }
            unT.setText("");

        }
        }
     }


}

}
