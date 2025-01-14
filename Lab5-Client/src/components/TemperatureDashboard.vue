<template>
  <div class="temperature-dashboard">
    <h1>Температурный мониторинг</h1>


    <div class="date-picker">
      <label for="startDate">Начальная дата:</label>
      <input type="date" id="startDate" v-model="startDate" />

      <label for="endDate">Конечная дата:</label>
      <input type="date" id="endDate" v-model="endDate" />


      <button @click="fetchTemperatureData">Получить данные</button>
    </div>


    <div class="current-temperature">
      <h2>Текущая температура:</h2>
      <p>{{ currentTemperature }} °C</p>
    </div>


    <div class="average-temperature">
      <h2>Средняя температура за выбранный период:</h2>
      <p>{{ averageTemperature }} °C</p>
    </div>


    <TemperatureChart
      :chartData="chartData"
      :startDate="startDate"
      :endDate="endDate"
    />


    <div v-if="error" class="error-message">
      <p>Ошибка загрузки данных. Пожалуйста, попробуйте снова.</p>
    </div>
  </div>
</template>

<script>
import axios from "axios";
import TemperatureChart from "./TemperatureChart.vue";

export default {
  name: "TemperatureDashboard",
  components: {
    TemperatureChart,
  },
  data() {
    return {
      currentTemperature: null,
      averageTemperature: null,
      chartData: null,
      error: false,
      startDate: "2025-01-14",
      endDate: "2025-01-17",
    };
  },
  mounted() {
    this.fetchTemperatureData();
  },
  methods: {
    async fetchTemperatureData() {
      try {
        const response = await axios.get(
          `http://127.0.0.1:8080/api/temperature/get?startDate=${this.startDate}&endDate=${this.endDate}`
        );
        const data = response.data;

        if (data.length === 0) {
          this.error = true;
          console.warn("Нет данных за выбранный период.");
          return;
        }

        const temperatures = data.map(item => item.temperature);
        const timestamps = data.map(item => 
          new Date(item.timestamp * 1000).toLocaleTimeString()
        );


        this.chartData = {
          labels: timestamps,
          datasets: [
            {
              label: "Температура (°C)",
              data: temperatures,
              fill: false,
              borderColor: "rgba(75, 192, 192, 1)",
              tension: 0.1,
            },
          ],
        };


        this.currentTemperature = temperatures[temperatures.length - 1];


        const totalTemperature = temperatures.reduce((sum, temp) => sum + temp, 0);
        this.averageTemperature = (totalTemperature / temperatures.length).toFixed(2);

        this.error = false;
      } catch (error) {
        console.error("Ошибка при получении данных:", error);
        this.chartData = null;
        this.error = true;
      }
    },
  },
};
</script>


<style scoped>
.temperature-dashboard {
  padding: 20px;
}

.date-picker {
  margin-bottom: 20px;
}

.date-picker label {
  margin-right: 10px;
}

.date-picker input {
  margin-right: 20px;
}

.current-temperature,
.average-temperature {
  margin: 20px 0;
}

.error-message {
  color: red;
  font-weight: bold;
}
</style>
