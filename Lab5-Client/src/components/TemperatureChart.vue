<template>
  <div class="temperature-chart-wrapper">
    <h2 v-if="chartData">График температуры за период с {{ startDate }} по {{ endDate }}:</h2>
    <p v-else>Данные для графика отсутствуют.</p>

    <div class="temperature-chart">
      <Line v-if="chartData" :data="chartData" />
    </div>
  </div>
</template>

<script>
import { Line } from "vue-chartjs";
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  Title,
  Tooltip,
  Legend,
  LineElement,
  PointElement,
  Filler,
} from "chart.js";

ChartJS.register(
  CategoryScale,
  LinearScale,
  Title,
  Tooltip,
  Legend,
  LineElement,
  PointElement,
  Filler
);

export default {
  name: "TemperatureChart",
  components: {
    Line,
  },
  props: {
    chartData: {
      type: Object,
      default: null,
    },
    startDate: {
      type: String,
      required: true,
    },
    endDate: {
      type: String,
      required: true,
    },
  },
};
</script>

<style scoped>
.temperature-chart-wrapper {
  display: flex;
  flex-direction: column;
  align-items: center;
  width: 100%;
  height: 60vh;
}

.temperature-chart {
  width: 60%;
  height: 100%;
}
</style>