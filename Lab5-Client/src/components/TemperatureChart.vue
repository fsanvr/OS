<template>
  <div class="temperature-chart-wrapper">
    <h2 v-if="chartData">График температуры за период с {{ startDate }} по {{ endDate }}:</h2>
    <p v-else>Данные для графика отсутствуют.</p>
    <!-- Рисуем график только при наличии данных -->
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
      default: null, // Устанавливаем значение по умолчанию
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
  flex-direction: column;   /* Устанавливаем вертикальное направление (первый элемент - заголовок, второй - график) */
  align-items: center;      /* Центрируем по горизонтали все элементы (включая заголовок и график) */
  width: 100%;
  height: 60vh;             /* Контейнер занимает 60% от высоты экрана */
}

.temperature-chart {
  width: 60%;   /* График будет занимать 80% ширины родительского контейнера */
  height: 100%; /* График будет занимать всю высоту контейнера */
}
</style>